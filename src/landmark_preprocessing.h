
#ifndef ALT_LANDMARK_PREPROCESSING_H
#define ALT_LANDMARK_PREPROCESSING_H

#include "alt/alt.h"
#include "graph.h"
#include <stddef.h>

/**
 * @brief Stores precomputed directed shortest-path distances for selected
 *        landmark vertices.
 *
 * @details Contains two independently allocated, contiguous distance tables.
 *          Each table uses row-major storage, with one row per landmark and
 *          one column per graph vertex.
 *
 *          For landmark position i and vertex v, the array offset is:
 *
 *              i * vertex_count + v
 *
 *          The landmark position refers to the selected landmark array's
 *          ordering, not the landmark's vertex ID.
 *
 *          distance_from stores d(l, v), the shortest distance from landmark
 *          l to vertex v. distance_to stores d(v, l), the shortest distance
 *          from vertex v to landmark l.
 *
 *          Unreachable distances are represented by positive infinity.
 *
 *          This structure owns both distance arrays, which must be released
 *          using landmark_distances_destroy(). It does not own the original
 *          graph or the selected landmark array.
 *
 *          A zero-initialized structure owns no allocations and can be used
 *          as a preprocessing destination.
 *
 * @par Invariants
 * - landmark_count does not exceed vertex_count.
 * - Each nonempty distance table contains landmark_count * vertex_count
 *   double values.
 * - Both distance arrays are non-NULL when landmark_count > 0.
 * - Both distance arrays are NULL when landmark_count == 0.
 * - Stored distances are finite and nonnegative, or positive infinity.
 */
typedef struct {
  /** @brief Number of selected landmarks represented in the tables. */
  size_t landmark_count;

  /** @brief Number of graph vertices represented in each landmark row. */
  size_t vertex_count;

  /**
   * @brief Shortest distances from each landmark to every graph vertex.
   *
   * @details Contains d(l, v), computed on the original directed graph.
   *          Indexed by landmark position and vertex ID.
   */
  double *distance_from;

  /**
   * @brief Shortest distances from every graph vertex to each landmark.
   *
   * @details Contains d(v, l), computed by running Dijkstra from each
   *          landmark on the reversed directed graph.
   *          Indexed by landmark position and vertex ID.
   */
  double *distance_to;
} alt_landmark_distances;

/**
 * @brief Precomputes directed shortest-path distances for selected landmarks.
 *
 * @details Uses the original directed CSR graph and a temporary reversed
 *          CSR graph to calculate distances in both directions for every
 *          selected landmark.
 *
 *          For each landmark, Dijkstra runs once on the original graph to
 *          obtain distances from that landmark and once on the reversed
 *          graph to obtain distances to that landmark.
 *
 *          The caller retains ownership of the graph and landmark array.
 *          The supplied landmark IDs must already be valid, distinct graph
 *          vertices, as guaranteed by successful landmark selection.
 *
 *          The output destination must be zero-initialized and must not
 *          already own allocated memory. On success, it receives ownership
 *          of both completed distance tables.
 *
 *          When landmark_count is zero, preprocessing succeeds without
 *          allocating distance tables or constructing a reversed graph.
 *          The landmarks pointer may be NULL in this case. Vertex count is
 *          still preserved.
 *
 *          On failure, all temporary and partially constructed allocations
 *          are released, and the output destination remains unchanged.
 *          The original graph and landmark array are never modified.
 *
 * @param[in] graph
 *     Pointer to a valid original directed CSR graph.
 *
 * @param[in] landmarks
 *     Array containing landmark_count distinct, valid vertex IDs.
 *     May be NULL when landmark_count is zero.
 *
 * @param[in] landmark_count
 *     Number of selected landmarks to preprocess. Must not exceed the
 *     graph's vertex count.
 *
 * @param[out] out_distances
 *     Pointer to a zero-initialized structure receiving the completed
 *     distance tables.
 *
 * @retval ALT_OK
 *     Preprocessing completed successfully, including zero-landmark
 *     preprocessing.
 * @retval ALT_INVALID_ARGUMENT
 *     A required pointer is NULL or an input precondition is violated.
 * @retval ALT_OUT_OF_MEMORY
 *     A required allocation failed or an allocation size cannot be
 *     represented.
 * @retval ALT_NUMERIC_ERROR
 *     A numerical failure occurred during a Dijkstra calculation.
 */
alt_status landmark_preprocess(const alt_graph *graph,
                               const alt_vertex_id *landmarks,
                               size_t landmark_count,
                               alt_landmark_distances *out_distances);

/**
 * @brief Releases all memory owned by precomputed landmark-distance tables.
 *
 * @details Frees both distance arrays and resets the structure to its
 *          zero-initialized state.
 *
 *          Passing NULL is permitted. Destroying a zero-initialized,
 *          partially initialized, or previously destroyed structure is safe.
 *
 *          The original graph and selected landmark array are not affected.
 *
 * @param[in,out] distances
 *     Pointer to the distance-table structure to destroy, or NULL.
 */
void landmark_distances_destroy(alt_landmark_distances *distances);

#endif // ALT_LANDMARK_PREPROCESSING_H
