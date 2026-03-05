# Multi-vector DEG Support

## Overview

The DEG project supports **multiple vectors** (N >= 2) controlled by a hyperparameter `num_vectors`. When `num_vectors == 2`, behavior is **unchanged** and backward compatible. For N > 2, the data model, loading, combined distance, R-tree, dual-index baselines, and parameter handling all work. DEG-specific algorithms (skyline, prune, route) that assume exactly two objectives remain as stubs for N > 2.

## Hyperparameter

- **`num_vectors`** (unsigned, default **2**)
  - Read from `Parameters`: `parameters.get<unsigned>("num_vectors")`.
  - Set on `Index` via `setNumVectors()`; used in load, combined distance, R-tree, and dual-index.
  - CLI: N=2 format auto-detected. For N > 2, pass `num_vectors` as third arg, comma-separated max distances as fourth arg, and comma-separated weights (summing to 1) as fifth arg.

## Data model

### N = 2 (current)

- Base: `base_emb_data_`, `base_loc_data_`; query: `query_emb_data_`, `query_loc_data_`.
- Dims: `base_emb_dim_`, `base_loc_dim_`, `query_emb_dim_`, `query_loc_dim_`.
- One scalar weight per query: `query_alpha_` (alpha in [0, 1]); combined distance = `alpha * e_d + (1 - alpha) * s_d`.

### N > 2

- **Vectors**: `base_vecs_`, `query_vecs_` (each `std::vector<float*>` of length N); `base_dims_`, `query_dims_` (length N).
- **Unified access**: `getBaseVecData(i)`, `getBaseVecDim(i)`, `getQueryVecData(i)`, `getQueryVecDim(i)`. For N = 2 and i in {0,1}, these return the existing emb/loc pointers and dims.
- **Query weights**: `query_weights_` (row-major `query_len_ * num_vectors_`); `getQueryWeight(query_id, vector_id)`.
- **Index constructor**: `Index(unsigned num_vectors, std::vector<float> const& max_distances)` supports N >= 2 and creates per-vector distance objects.

## Distance

- **Combined distance** (in `index.h`):
  - `combined_distance(index, e_d, s_d)` uses `index->get_alpha()` for N = 2.
  - `combined_distance(index, query_id, e_d, s_d)` and `combined_distance(index, query_id, dists)` for per-query weights and N > 2.
- **E_Distance**: Unchanged; dimension-agnostic.
- **S_Distance**: Now dimension-agnostic (loops over `length` parameter). Previously hardcoded 2D.

## Load

- **N = 2**: Unchanged. Two base files, two query files, one alpha file; `ComponentLoad::LoadInner(...)`.
- **N > 2**: `LoadInner` loads N base vector files and N query vector files from parameters (`base_vec_path_0`, ..., `base_vec_path_{N-1}`, `query_vec_path_0`, ..., `query_vec_path_{N-1}`), plus a query weights file (`query_weights_path`) with N floats per query. Ground truth loaded as before. Legacy emb/loc pointers are set from the first two vectors for backward compatibility.

### File naming convention (N > 2)

- Base: `{dataset_dir}/base_vec_0.fvecs`, `base_vec_1.fvecs`, ..., `base_vec_{N-1}.fvecs`
- Query: `{dataset_dir}/query_vec_0.fvecs`, `query_vec_1.fvecs`, ..., `query_vec_{N-1}.fvecs`
- Weights: `{dataset_dir}/query_weights.fvecs` (fvecs with dim = num_vectors)
- Ground truth: unchanged (`range_{range}_top10_results.ivecs`)

## R-tree

- **Generalized beyond 2D**: `RTreeIndexBase` abstract class with `RTreeIndexND<DIMS>` template instantiated for dimensions 2--16. Factory `makeRTreeIndex(dims)` creates the correct instantiation at runtime.
- `ComponentInitRTree::InitInner` uses `index->getBaseLocDim()` to determine R-tree dimension.
- Legacy 2D `RTreeIndex` preserved and inherits from `RTreeIndexBase`.
- In Index: `RTreeIndexBase *rtree_index_` with `initRTree(dims)` and `get_R_Tree()`.

## Dual-index baselines

- **N = 2**: Unchanged. Two indices with alpha=0 and alpha=1 (`final_index_1`, `final_index_2`).
- **N > 2**: N dual indices (`dual_indices_`), each built with emb data pointing to one vector (alpha=1). At search time, all N indices are queried, results merged and re-ranked with `combined_distance(ref, query_id, dists)` using per-query weights.
- `IndexBuilder` has a multi-vector constructor: `IndexBuilder(num_threads, num_vectors, max_distances, dual_index)`.
- `load_graph(TYPE, std::vector<std::string>)` overload for loading N graph files.

## Algorithms

### Implemented for N = 2

- HNSW, DEG (build, prune, route, entry), R-tree (any dim), BS4, baselines 1--4, dual-index (baseline2/3).

### Generalized for N > 2

- **Data layer**: `num_vectors_`, unified getters/setters, query weights, `base_vecs_`/`query_vecs_`/dims.
- **Combined distance**: Weighted sum of per-vector distances via `combined_distance(idx, query_id, dists)`.
- **Load**: N vector files + query weights file loaded via numbered paths.
- **set_data_path**: Generates numbered paths for each dataset when N > 2.
- **S_Distance**: Generalized to arbitrary dimensions.
- **R-tree**: Supports dimensions 2--16 via `RTreeIndexND<DIMS>` template.
- **Dual-index search**: N-index merge with combined distance re-ranking.
- **HNSW build**: Works for single-vector indices in the dual-index approach (each dual index uses one vector as "emb" with alpha=1).

### TODO (stubs)

- **DEG** (skyline, convex hull, 2D pruning, entry, route): Early return in `ComponentInitDEG::InitInner`, `ComponentSearchRouteDEG::RouteInner`, `ComponentDEGPruneHeuristic::PruneInner` when `index->getNumVectors() != 2`.
- **Ground truth path for N > 2**: `set_data_path` in `include/set_para.h` maps alpha to a fixed set of range values `{0, 0.1, 0.3, 0.5, 0.7, 0.9, 1.0}` and exits on anything else. For N > 2, alpha is hardcoded to 0.5 (range 3), so the ground truth file is always `range_3_top10_results.ivecs`. A proper N > 2 ground truth selection mechanism (e.g., accepting a range directly, or a weight-based naming convention) is not yet implemented.

## File change list

| File | Changes |
|------|--------|
| `include/set_para.h` | `get_num_vectors()` helper; `set_data_path` generates numbered paths for N > 2, sets legacy + numbered parameter keys. |
| `include/index.h` | `num_vectors_`, `base_vecs_`/`query_vecs_`/`base_dims_`/`query_dims_`, `query_weights_`, `vec_dists_`; getters/setters; `Index(num_vectors, max_distances)`; `combined_distance()` helpers; `RTreeIndexBase *rtree_index_`; `initRTree(dims)`. |
| `include/distance.h` | `S_Distance::compare` generalized to loop over `length` (no longer 2D only). |
| `include/rtree.h` | `RTreeIndexBase` abstract class; `RTreeIndex` (2D, inherits base); `RTreeIndexND<DIMS>` template (2--16); `makeRTreeIndex(dims)` factory. |
| `include/builder.h` | Multi-vector constructor `IndexBuilder(n_threads, num_vectors, max_distances, dual)`; `dual_indices_` vector; `load_graph` overload for N files. |
| `src/component_load.cpp` | N > 2 branch: loads N base/query vector files + query weights from numbered parameter keys. |
| `src/builder.cpp` | Dual load for N > 2: creates N indices with per-vector emb override. Dual search for N > 2: searches all N indices, merges with combined_distance. N-file `load_graph` overload. |
| `src/component_conn.cpp` | Replace inline formula with `combined_distance(index, e_d, s_d)`. |
| `src/component_route.cpp` | Same; DEG `RouteInner`: TODO stub when N != 2. |
| `src/component_prune.cpp` | Same; DEG `PruneInner`: TODO stub when N != 2. |
| `src/component_init.cpp` | `ComponentInitRTree::InitInner` generalized: uses `initRTree(loc_dim)` and loops over dimensions. DEG `InitInner`: TODO stub when N != 2. |
| `src/component_search_entry.cpp` | Replace with `combined_distance`. |
| `src/rtree.cpp` | Unchanged (2D `RTreeIndex` implementations preserved). |
| `test/main.cpp` | Auto-detects N=2 vs N > 2 CLI format. N > 2: `./main alg dataset N max_d0,...,max_dN alpha exc_type`. |

## How to run

- **N = 2**: Unchanged. Example:
  `./main deg openimage 0.5 1 1 build`
  (algorithm, dataset, alpha, max_spatial_distance, max_emb_distance, exc_type).

- **N > 2**: Example:
  `./main baseline2 openimage 3 1.0,1.0,1.0 0.33,0.33,0.34 build`
  (algorithm, dataset, num_vectors, comma-separated max_distances, comma-separated weights summing to 1, exc_type).
  - **set_data_path** generates numbered paths: `base_vec_0.fvecs`, ..., `query_vec_0.fvecs`, ..., `query_weights.fvecs`.
  - **LoadInner** loads N vector files and query weights.
  - **HNSW build/search**: Works. Each dual index is built on one vector.
  - **DEG build/search**: Stub (early return when N != 2).
  - **R-tree**: Works for any spatial dimension 2--16 via `RTreeIndexND<DIMS>`.
