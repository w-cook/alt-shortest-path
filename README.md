# ALT Shortest-Path Library

A C17 library in development for ALT (A*, Landmarks, Triangle Inequality)
shortest-path queries on weighted directed graphs. The intended public API
will support reusable landmark preprocessing, distance queries, and path
reconstruction.

**Project status (November 7, 2026):** The internal compressed sparse row
(CSR) graph, reverse CSR construction, binary min-heap priority queue,
single-source Dijkstra shortest-distance algorithm, three landmark-selection
strategies (explicit, seeded random, and farthest-first), and directed
landmark-distance preprocessing are implemented and tested. ALT/A* queries and the public index
lifecycle remain planned. **The public ALT functions are declared but not yet
implemented; this is not a usable shortest-path library release.**

## Implemented Components

- **CSR graph (`src/graph.c`, `src/graph.h`):** validates and copies directed,
  finite, nonnegative-weight edges into a compact representation, preserving
  original input edge IDs. Supports parallel edges, self-loops, isolated
  vertices, and zero-weight edges. Also constructs independently owned reverse
  CSR graphs while preserving edge weights and original IDs.
- **Binary min-heap (`src/priority_queue.c`, `src/priority_queue.h`):** orders
  entries by priority, preserves vertex IDs and recorded distances, allows
  duplicate entries, and grows storage dynamically. Rejects nonfinite
  priorities and recorded distances; supports destruction and reuse.
- **Dijkstra (`src/dijkstra.c`, `src/dijkstra.h`):** calculates shortest
  distances from one source to all vertices in the internal directed CSR graph.
  Uses the binary min-heap with duplicate entries and stale-entry detection;
  preserves positive infinity for unreachable vertices. Reports detected
  distance overflow and positive-weight rounding stagnation as numerical
  errors. It is an internal preprocessing building block, not a public path
  query or path-reconstruction API.
- **Landmark selection (`src/landmark_selection.c`,
  `src/landmark_selection.h`):** validates and copies explicit landmark IDs;
  selects distinct random vertices with seeded SplitMix64 and unbiased bounded
  sampling; and performs deterministic farthest-first selection using weighted
  shortest-path distances on a temporary undirected graph projection. The
  farthest-first method uses lowest-vertex-ID tie-breaking and prioritizes
  uncovered connected components. Selection returns an independently owned array.
- **Directed landmark-distance preprocessing (`src/landmark_preprocessing.c`,
  `src/landmark_preprocessing.h`):** computes `d(l, v)` on the original directed
  CSR graph and `d(v, l)` using a temporary reversed graph for each selected
  landmark. Stores the two results in independently owned, contiguous row-major
  `double` tables. Supports unreachable distances, zero landmarks without table
  allocations, checked allocation sizes, and cleanup on errors. This internal
  component does not construct the public ALT index or evaluate the heuristic.
- **Tests:** CTest runs five executables: `graph_tests` (21 test functions),
  `priority_queue_tests` (17), `dijkstra_tests` (17),
  `landmark_selection_tests` (29), and `landmark_preprocessing_tests` (19). The
  graph tests include reverse CSR correctness, edge cases, ownership, and
  double reversal. The Dijkstra tests cover directed shortest paths, repeated
  calls, graph edge cases, heap growth, invalid arguments, numerical boundaries,
  and reversed-graph distance integration. Landmark-selection tests cover
  explicit and seeded random selection, weighted farthest-first behavior,
  deterministic ties, disconnected components, undirected projection, and
  configuration validation. Preprocessing tests cover directed forward/reverse
  distances, landmark ordering, graph edge cases, argument validation,
  numerical-error propagation, and ownership/destruction. All five executables
  passed local Debug, Release (with assertions enabled), AddressSanitizer (with
  leak detection), and UndefinedBehaviorSanitizer validation on November 7,
  2026: **103 test functions across five CTest executables**, not 103 separate
  CTest tests.

## Planned Capabilities

- Directed ALT heuristic evaluation and unidirectional ALT/A* search on
  immutable graph snapshots
- Distance-only queries and complete paths with original input edge IDs
- Correctness testing against independent Dijkstra baselines and performance
  benchmarks measuring preprocessing costs and repeated-query tradeoffs

## Build and Test the Implemented Components

With CMake and a C17 compiler installed, run from the repository root:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

These commands build the internal static library and run `graph_tests`,
`priority_queue_tests`, `dijkstra_tests`, `landmark_selection_tests`, and
`landmark_preprocessing_tests`. They do **not** produce a working public ALT
query interface yet.

## Technology

- C17
- CMake
- CTest

## Design Documentation

- [Architecture](docs/architecture.md)
- [Algorithm specification](docs/algorithm-specification.md)
- [Public C API specification (draft)](docs/api-specification.md)

## References

Goldberg, A. V., and Harrelson, C. (2005).  
_Computing the Shortest Path: A* Search Meets Graph Theory._
