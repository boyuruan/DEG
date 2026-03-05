# DEG Usage Path

This document describes the full usage path for the Dynamic Edge navigation Graph (DEG) index: from application entry point through build and search.

## Full DEG usage path (figure)

```mermaid
flowchart TB
    subgraph entry [Application entry]
        main["main()"]
        DEG_fn["DEG(parameters)"]
        main --> DEG_fn
    end

    subgraph build [Build path: exc_type = build]
        B1["IndexBuilder(num_threads, max_emb_dist, max_spatial_dist)"]
        B2["builder->load(base_emb, base_loc, query_emb, query_loc, query_alpha, ground, parameters)"]
        B3["builder->init(INIT_DEG)"]
        B4["builder->save_graph(INDEX_DEG, graph_file)"]
        B1 --> B2 --> B3 --> B4
    end

    subgraph search [Search path: exc_type = search]
        S1["builder->load(...)"]
        S2["builder->load_graph(INDEX_DEG, graph_file)"]
        S3["builder->search(SEARCH_ENTRY_NONE, ROUTER_DEG, L_SEARCH_ASCEND, parameters)"]
        S1 --> S2 --> S3
    end

    DEG_fn --> build
    DEG_fn --> search

    subgraph load_detail [Load: ComponentLoad::LoadInner]
        L1["Load base_emb, base_loc (fvecs)"]
        L2["Load query_emb, query_loc, query_alpha (fvecs)"]
        L3["Load ground truth (ivecs)"]
        L4["Index: setBaseEmbData, setBaseLocData, setQuery*, setGroundData, setParam"]
        L1 --> L2 --> L3 --> L4
    end

    B2 --> load_detail

    subgraph init_deg [Init DEG: ComponentInitDEG::InitInner]
        I1["SetConfigs (max_m, ef_construction, n_threads)"]
        I2["BuildByIncrementInsert"]
        I1 --> I2
    end

    B3 --> init_deg

    subgraph build_increment [BuildByIncrementInsert]
        BI1["Resize DEG_nodes_, create first DEGNode"]
        BI2["DEG_enterpoints.push_back(first)"]
        BI3["EntryInner: compute emb_center, loc_center"]
        BI4["Parallel for i = 1..base_len: InsertNode(qnode, visited_list)"]
        BI1 --> BI2 --> BI3 --> BI4
    end

    I2 --> build_increment

    subgraph insert_node [InsertNode per node]
        IN1["SearchAtLayer(qnode, visited_list, pool)"]
        IN2["ComponentDEGPruneHeuristic::PruneInner(pool, range, result)"]
        IN3["Link(qnode, result neighbors)"]
        IN4["UpdateEnterpointSet(qnode)"]
        IN1 --> IN2 --> IN3 --> IN4
    end

    BI4 --> insert_node

    subgraph search_inner [SearchAtLayer build-time]
        SL1["skyline_queue.init_queue(insert_points)"]
        SL2["findSkyline / skyline_result"]
        SL3["Prune: pick neighbors by alpha intervals"]
        SL1 --> SL2 --> SL3
    end

    IN1 --> search_inner

    subgraph save_deg [save_graph INDEX_DEG]
        SV1["Write enterpoint_set size and node ids"]
        SV2["For each node: node_id, neighbor_size, neighbor_id + use_range per neighbor"]
        SV1 --> SV2
    end

    B4 --> save_deg

    subgraph load_graph_deg [load_graph INDEX_DEG]
        LG1["Resize DEG_nodes_, create DEGNodes"]
        LG2["Read enterpoint_set (ids)"]
        LG3["For each node: read neighbors + active_range (int8_t pairs)"]
        LG4["SetSearchFriends(neighbors)"]
        LG1 --> LG2 --> LG3 --> LG4
    end

    S2 --> load_graph_deg

    subgraph search_phase [search: per query]
        SP1["set_alpha(query_alpha[i])"]
        SP2["ComponentSearchEntryNone::SearchEntryInner(i, pool)"]
        SP3["ComponentSearchRouteDEG::RouteInner(i, pool, res[i])"]
        SP1 --> SP2 --> SP3
    end

    S3 --> search_phase

    subgraph route_deg [RouteInner DEG]
        R1["VisitedList, priority_queue DEG_FurtherFirst / DEG_CloserFirst"]
        R2["SearchAtLayer(query, DEG_enterpoint_, 0, visited_list, result)"]
        R3["Iterate enterpoint_set: compute e_d, s_d, combined_distance; fill candidates"]
        R4["Greedy expand: pop candidate, for each neighbor check alpha in use_range"]
        R5["If combined_distance better than threshold, add to result"]
        R6["Copy top-K from tmp to res"]
        R1 --> R2 --> R3 --> R4 --> R5 --> R6
    end

    SP3 --> route_deg

    subgraph recall [After search loop]
        RC1["Compare res with ground truth"]
        RC2["Compute K-NN accuracy / recall"]
        RC1 --> RC2
    end

    search_phase --> RC1
```

## Summary

| Phase | Key components |
|-------|----------------|
| **Entry** | `main()` → `DEG(parameters)` (test/main.cpp). |
| **Load** | `ComponentLoad::LoadInner`: base/query emb & loc (fvecs), query_alpha, ground (ivecs) → Index. |
| **Build** | `ComponentInitDEG::InitInner` → `SetConfigs`, `BuildByIncrementInsert` → `EntryInner`, `InsertNode` (parallel), `SearchAtLayer` (skyline_queue, findSkyline), `ComponentDEGPruneHeuristic::PruneInner`, `Link`, `UpdateEnterpointSet`. |
| **Save** | `save_graph(INDEX_DEG)`: enterpoint set, then per-node neighbors with alpha `use_range`. |
| **Load graph** | `load_graph(INDEX_DEG)`: restore DEG_nodes_, enterpoint_set, neighbors + active_range. |
| **Search** | For each query: `set_alpha` → `SearchEntryInner` (NONE = empty pool) → `ComponentSearchRouteDEG::RouteInner` → `SearchAtLayer` over enterpoint_set, greedy expansion with alpha-in-range check → top-K to res. Recall vs ground truth. |

## Data flow (two vectors)

- **Embedding**: `base_emb_data_`, `query_emb_data_`; distance via `E_Distance`.
- **Spatial**: `base_loc_data_`, `query_loc_data_`; distance via `S_Distance` (2D).
- **Combined**: `alpha * e_d + (1 - alpha) * s_d` via `stkq::combined_distance(index, e_d, s_d)` (or per-query weight).
- **DEG graph**: Each node stores neighbors with an **available_range** (alpha intervals) for when the edge is useful; search uses **active_range** (int8_t) for alpha-in-range pruning during route.
