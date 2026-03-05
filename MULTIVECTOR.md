# Multi-vector DEG Support

## Overview

The DEG project has been extended to **prepare for multiple vectors** (N ≥ 2) controlled by a hyperparameter `num_vectors`. When `num_vectors == 2`, behavior is **unchanged** and backward compatible. For N > 2, the data model, combined distance, and parameter handling are in place; algorithms that assume exactly two objectives (e.g. DEG skyline, dual-index baselines, 2D R-tree) are left as stubs and marked TODO.

## Hyperparameter

- **`num_vectors`** (unsigned, default **2**)
  - Read from `Parameters`: `parameters.get<unsigned>("num_vectors")` (callers use default 2 when the key is absent, e.g. via a helper in `set_para.h`).
  - Set on `Index` via `setNumVectors()`; used in load, combined distance, and algorithm stubs.
  - In **test/main.cpp**, `parameters.set<unsigned>("num_vectors", 2)` is set explicitly; change this to test N > 2 (path logic and several algorithms will then hit TODO/exit).

## Data model

### N = 2 (current)

- Base: `base_emb_data_`, `base_loc_data_`; query: `query_emb_data_`, `query_loc_data_`.
- Dims: `base_emb_dim_`, `base_loc_dim_`, `query_emb_dim_`, `query_loc_dim_`.
- One scalar weight per query: `query_alpha_` (alpha ∈ [0, 1]); combined distance = `alpha * e_d + (1 - alpha) * s_d`.

### N > 2 (prepared)

- **Vectors**: `base_vecs_`, `query_vecs_` (each `std::vector<float*>` of length N); `base_dims_`, `query_dims_` (length N).
- **Unified access**: `getBaseVecData(i)`, `getBaseVecDim(i)`, `getQueryVecData(i)`, `getQueryVecDim(i)`. For N = 2 and i ∈ {0,1}, these return the existing emb/loc pointers and dims.
- **Query weights**: For N > 2, `query_weights_` (row-major `query_len_ * num_vectors_`); `getQueryWeight(query_id, vector_id)`.
- **Index constructor**: `Index(float max_emb_dist, float max_spatial_dist)` sets `num_vectors_ = 2`. `Index(unsigned num_vectors, std::vector<float> const& max_distances)` supports N ≥ 2 and creates per-vector distance objects.

## Distance

- **Combined distance** (in `index.h`):
  - `combined_distance(index, e_d, s_d)` uses `index->get_alpha()` for N = 2.
  - `combined_distance(index, query_id, e_d, s_d)` and `combined_distance(index, query_id, dists)` for per-query weights and N > 2.
- **E_Distance**: Unchanged; dimension-agnostic.
- **S_Distance**: **2D only** (a[0], a[1]). Comment in `distance.h`: *Multi-vector: S_Distance is 2D only; additional vector dimensions TODO.*

## Load

- **N = 2**: Unchanged. Two base files, two query files, one alpha file; `ComponentLoad::LoadInner(..., data_emb_file, data_loc_file, ...)`.
- **N > 2**: **TODO.** `LoadInner` asserts `num_vectors == 2`. Intended extension: path lists for base/query vectors and a query weight file (N floats per query); see plan.

## Algorithms

### Implemented for N = 2

- HNSW, DEG (build, prune, route, entry), R-tree (2D spatial), BS4, baselines 1–4, dual-index (baseline2/3).

### Generalized for N > 2

- **Data layer**: `num_vectors_`, unified getters/setters, query weights, optional `base_vecs_`/`query_vecs_`/dims.
- **Combined distance**: Helper used everywhere instead of hardcoded `alpha * e_d + (1 - alpha) * s_d`; N > 2 uses weighted sum of per-vector distances when weights are available.
- **Parameters / set_para**: `num_vectors` read with default 2; `set_data_path` exits with message if N > 2 (path logic TODO).

### TODO (stubs)

- **DEG** (skyline, convex hull, 2D pruning, entry, route): Early return in `ComponentInitDEG::InitInner`, `ComponentSearchRouteDEG::RouteInner`, `ComponentDEGPruneHeuristic::PruneInner` when `index->getNumVectors() != 2`; comment *TODO: multi-vector (N>2) not implemented*.
- **Dual-index baselines (baseline2, baseline3)**: In `IndexBuilder::load(..., dual = true)`, if `num_vectors > 2` then exit with message *Dual-index build/search for num_vectors>2 is not implemented*.
- **R-tree**: `rtree.h`: *Multi-vector: R-tree remains 2D; TODO: support multiple spatial dimensions / high-dim R-tree.* `RTreeIndex`: *Uses first non-embedding (spatial) vector only; 2D.*
- **S_Distance**: 2D only; see **Distance** above.
- **set_para / set_data_path**: Path logic for N > 2 (extra vector files, weight file layout) not implemented; exits when N > 2.

## File change list

| File | Changes |
|------|--------|
| `include/set_para.h` | `get_num_vectors()` helper (default 2); `set_data_path` exits if N > 2 with TODO message. |
| `include/index.h` | `num_vectors_`, `base_vecs_`/`query_vecs_`/`base_dims_`/`query_dims_`, `query_weights_`, `vec_dists_`; getters/setters; `Index(num_vectors, max_distances)`; `combined_distance()` helpers. |
| `include/distance.h` | Comment: S_Distance 2D only; multi-vector TODO. |
| `include/rtree.h` | Comments: NumSpaceDims 2; RTreeIndex 2D / first spatial vector only; TODO multi-dim. |
| `include/builder.h` | No signature change; dual + N>2 handled in .cpp. |
| `src/component_load.cpp` | Read `num_vectors` from parameters, set on index; assert `num_vectors == 2` (N>2 load TODO). |
| `src/builder.cpp` | Dual load: exit if `num_vectors > 2`; replace `alpha*e_d + (1-alpha)*s_d` with `combined_distance(...)`; set_alpha before dual search loops. |
| `src/component_conn.cpp` | Replace inline formula with `combined_distance(index, e_d, s_d)`. |
| `src/component_route.cpp` | Same; DEG `RouteInner`: TODO stub when N != 2. |
| `src/component_prune.cpp` | Same; DEG `PruneInner`: TODO stub when N != 2. |
| `src/component_init.cpp` | Replace with `combined_distance`; DEG `InitInner`: TODO stub when N != 2. |
| `src/component_search_entry.cpp` | Replace with `combined_distance`. |
| `test/main.cpp` | `parameters.set<unsigned>("num_vectors", 2)`. |

## How to run

- **N = 2**: Unchanged. Example:  
  `./main deg openimage 0.5 1 1 build`  
  (algorithm, dataset, alpha, max_spatial_distance, max_emb_distance, exc_type).

- **N > 2**: Set `parameters.set<unsigned>("num_vectors", n)` (e.g. in main or via a future CLI flag). Currently:
  - **set_data_path** exits with *Multi-vector (N>2) path logic not implemented*.
  - If paths were extended, **LoadInner** would assert until N>2 load is implemented.
  - **DEG** init/route/prune and **dual-index** build/search exit or no-op when N > 2.

To actually run with N > 2, path setup, load logic, and the TODO algorithm stubs would need to be implemented as described in the plan.
