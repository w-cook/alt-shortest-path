#ifndef ALT_GRAPH_H
#define ALT_GRAPH_H

#include "alt/alt.h"
#include <stddef.h>

/**
 * @brief Compressed sparse row (CSR) representation of a directed,
 *        weighted graph.
 *
 * @details Stores a graph using contiguous, zero-based vertex identifiers
 *          and contiguous adjacency ranges for outgoing edges.
 *
 *          Parallel edges, self-loops, isolated vertices, and zero-weight
 *          edges are supported.
 *
 *          The graph owns its allocated arrays, which must be released
 *          using graph_destroy(). A zero-initialized structure owns no
 *          allocations and can be used as a construction destination.
 *
 * @par Invariants
 * - vertex_count is greater than zero for a constructed graph.
 * - edge_starts contains vertex_count + 1 elements.
 * - edge_starts[0] is zero.
 * - edge_starts[vertex_count] equals edge_count.
 * - edge_starts is monotonically nondecreasing.
 * - The edge-data arrays contain edge_count elements when edge_count > 0.
 * - All stored edge weights are finite and nonnegative.
 */
typedef struct {
  /** @brief Number of vertices in the graph. */
  size_t vertex_count;

  /** @brief Number of directed edges in the graph. */
  size_t edge_count;

  /**
   * @brief Starting offsets of each vertex's outgoing edge range.
   *
   * @details Contains vertex_count + 1 elements. The outgoing edges of
   *          vertex v occupy the half-open interval
   *          [edge_starts[v], edge_starts[v + 1]).
   *
   *          Commonly referred to as row offsets in CSR literature.
   */
  size_t *edge_starts;

  /**
   * @brief Destination vertex for each stored edge.
   *
   * @details Contains edge_count elements indexed by CSR edge position.
   */
  size_t *targets;

  /**
   * @brief Weight of each stored edge.
   *
   * @details Contains edge_count elements indexed by CSR edge position.
   *          All weights are finite and nonnegative.
   */
  double *weights;

  /**
   * @brief Original input edge identifier for each stored edge.
   *
   * @details Contains edge_count elements indexed by CSR edge position.
   *          Preserves original input edge identities through CSR
   *          construction and graph reversal.
   */
  size_t *original_edge_ids;
} alt_graph;

/**
 * @brief Constructs a CSR graph from a caller-provided directed edge list.
 *
 * @details Validates the input graph dimensions, vertex IDs, and edge weights,
 *          then constructs a compressed sparse row representation.
 *          Original input edge identities are preserved even when edges are
 *          reordered during construction.
 *
 *          The caller retains ownership of the input edge array and takes
 *          ownership of the graph's allocations upon success.
 *          The destination must be zero-initialized and must not already own
 *          allocated memory. It remains unchanged upon failure.
 *
 * @param[in] vertex_count
 *     Number of vertices in the graph. Must be greater than zero.
 *
 * @param[in] edges
 *     Caller-provided directed edge array. May be null when edge_count is zero.
 *
 * @param[in] edge_count
 *     Number of edges in the input array.
 *
 * @param[out] out_graph
 *     Caller-provided pointer to a zero-initialized graph receiving the
 *     constructed CSR representation.
 *
 * @retval ALT_OK
 *     Graph was successfully constructed.
 * @retval ALT_INVALID_ARGUMENT
 *     A required pointer is null.
 * @retval ALT_INVALID_GRAPH
 *     Graph dimensions, vertex IDs, or edge weights are invalid.
 * @retval ALT_OUT_OF_MEMORY
 *     A required allocation failed or its size cannot be represented.
 */
alt_status graph_build(size_t vertex_count, const alt_edge *edges,
                       size_t edge_count, alt_graph *out_graph);

/**
 * @brief Constructs a reversed graph in which every directed edge points in
 *        the opposite direction.
 *
 * @details Accepts an existing, valid CSR graph and constructs a second CSR
 *          graph with every edge reversed. Each edge's original weight and
 *          original_edge_ids value is preserved, along with vertex and edge
 *          counts. The original graph remains unchanged.
 *
 *          The caller retains ownership of the input graph and takes
 *          ownership of the reversed graph's allocations upon success.
 *          The destination must be zero-initialized, must not already own
 *          allocated memory, and must not alias the source.
 *          The destination remains unchanged upon failure.
 *
 * @param[in] graph
 *     Pointer to a valid CSR graph. The graph is not modified.
 *
 * @param[out] out_graph
 *     Caller-provided pointer to a zero-initialized graph receiving the
 *     reversed version of the original graph.
 *
 * @retval ALT_OK
 *     Graph was successfully reversed.
 * @retval ALT_INVALID_ARGUMENT
 *     A required pointer is null or the source and destination alias.
 * @retval ALT_OUT_OF_MEMORY
 *     A required allocation failed or its size cannot be represented.
 */
alt_status graph_build_reverse(const alt_graph *graph, alt_graph *out_graph);

/**
 * @brief Releases all memory owned by a CSR graph.
 *
 * @details Frees the graph's allocated arrays and resets the structure
 *          to its zero-initialized state.
 *
 *          Passing a null pointer is permitted. Destroying an already
 *          zero-initialized or previously destroyed graph is also safe.
 *
 * @param[in,out] graph
 *     Pointer to the graph to destroy, or null.
 */
void graph_destroy(alt_graph *graph);

#endif // ALT_GRAPH_H
