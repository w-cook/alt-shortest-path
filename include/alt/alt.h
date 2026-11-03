#ifndef ALT_H
#define ALT_H

#include <stddef.h>
#include <stdint.h>

typedef struct alt_index alt_index;

/**
 * @brief Identifies a vertex within a graph.
 *
 * @details Vertex identifiers are zero-based contiguous integers.
 *          For a graph containing vertex_count vertices, valid IDs
 *          range from zero through vertex_count - 1.
 */
typedef size_t alt_vertex_id;
/**
 * @brief Identifies an edge in the caller-provided graph.
 *
 * @details Edge identifiers correspond to zero-based positions in the
 *          original input edge array. For a graph containing edge_count
 *          edges, valid IDs range from zero through edge_count - 1.
 *
 *          Internal graph construction may reorder edges, but their
 *          original identifiers are preserved. This distinction is
 *          particularly important for graphs containing parallel edges.
 */
typedef size_t alt_edge_id;

/**
 * @brief Represents a directed, weighted graph edge.
 *
 * @details An edge connects a source vertex to a target vertex with
 *          a finite, nonnegative weight.
 *
 *          Both vertex identifiers must be valid for the graph being
 *          constructed. Self-loops, parallel edges, and zero-weight
 *          edges are supported.
 *
 *          Edges are directed. An undirected connection must be
 *          represented using two oppositely directed edges.
 *
 *          The edge's position in the caller-provided array determines
 *          its original edge identifier.
 */
typedef struct {
  /** @brief Identifier of the vertex from which the edge originates. */
  alt_vertex_id source;
  /** @brief Identifier of the vertex to which the edge points. */
  alt_vertex_id target;
  /**
   * @brief Nonnegative cost of traversing the edge.
   *
   * @details Must be finite and greater than or equal to zero.
   *          Negative values, NaN, and infinity are invalid.
   */
  double weight;
} alt_edge;

/**
 * @brief Specifies the strategy used to select landmark vertices.
 *
 * @details All selection strategies produce distinct, valid vertex
 *          identifiers. Landmark selection influences preprocessing
 *          costs and query performance, but does not change the
 *          mathematical correctness requirements of ALT.
 *
 *          Supported strategies are:
 *          - Explicit: use a caller-provided list of landmark IDs.
 *          - Random: sample vertices without replacement using a
 *            reproducible, library-local pseudorandom generator.
 *          - Farthest: select vertices using weighted distances in
 *            an undirected projection of the original graph.
 */
typedef enum {
  ALT_LANDMARK_EXPLICIT, /**< Use caller-specified landmark vertices. */
  ALT_LANDMARK_RANDOM, /**< Select distinct vertices using a deterministic seed.
                        */
  ALT_LANDMARK_FARTHEST /**< Select vertices using the farthest-first heuristic.
                         */
} alt_landmark_method;

/**
 * @brief Configuration for selecting landmarks during ALT index construction.
 *
 * @details The method field determines which additional configuration
 *          fields are used. Fields unrelated to the selected method
 *          are ignored.
 *
 *          The requested landmark count must not exceed the number
 *          of graph vertices. A count of zero is valid for all methods
 *          and produces an empty landmark set, allowing queries to
 *          operate with a zero heuristic.
 *
 *          For explicit selection, the caller supplies distinct,
 *          in-range vertex IDs. Random selection uses the supplied
 *          seed for reproducibility. Farthest-first selection uses
 *          the supplied anchor and the library's defined distance
 *          and tie-breaking policies.
 *
 *          The configuration and any caller-supplied vertex array
 *          remain owned by the caller.
 */
typedef struct {
  /** @brief Landmark-selection strategy to use. */
  alt_landmark_method method;
  /**
   * @brief Number of landmarks to select.
   *
   * @details Must not exceed the graph's vertex count.
   *          Zero is permitted for every selection method.
   */
  size_t count;
  /**
   * @brief Caller-provided landmark IDs for explicit selection.
   *
   * @details Must reference at least count valid, distinct vertex IDs
   *          when explicit selection requests a positive count.
   *          May be NULL when count is zero.
   *          Ignored by other selection methods.
   */
  const alt_vertex_id *vertices;
  /**
   * @brief Pseudorandom seed for random landmark selection.
   *
   * @details Identical seeds and vertex ordering produce identical
   *          selections for the same library implementation.
   *          The specific sequence is not guaranteed to remain
   *          unchanged across incompatible future library versions.
   *          Ignored by other selection methods.
   */
  uint64_t seed;
  /**
   * @brief Starting reference vertex for farthest-first selection.
   *
   * @details Must identify a valid graph vertex when farthest-first
   *          selection requests a positive landmark count.
   *          Ignored when count is zero or another method is used.
   */
  alt_vertex_id anchor;
} alt_landmark_options;

typedef enum {
  ALT_OK,
  ALT_NO_PATH,
  ALT_INVALID_ARGUMENT,
  ALT_INVALID_GRAPH,
  ALT_INVALID_CONFIGURATION,
  ALT_OUT_OF_MEMORY,
  ALT_NUMERIC_ERROR
} alt_status;

typedef struct {
  alt_vertex_id *vertices;
  alt_edge_id *edges;
  size_t vertex_count;
  size_t edge_count;
  double total_cost;
} alt_path;

alt_status alt_index_build(size_t vertex_count, const alt_edge *edges,
                           size_t edge_count,
                           const alt_landmark_options *options,
                           alt_index **out_index);

void alt_index_destroy(alt_index *index);

alt_status alt_distance(const alt_index *index, alt_vertex_id start,
                        alt_vertex_id target, double *out_distance);

alt_status alt_shortest_path(const alt_index *index, alt_vertex_id start,
                             alt_vertex_id target, alt_path **out_path);

void alt_path_destroy(alt_path *path);

#endif
