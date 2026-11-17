#ifndef ALT_INDEX_H
#define ALT_INDEX_H

#include "alt/alt.h"
#include "graph.h"
#include "landmark_preprocessing.h"

/**
 * @brief Opaque ALT index containing an immutable graph snapshot and
 *        precomputed landmark data.
 *
 * @details Owns the CSR representation of the original directed graph,
 *          the selected landmark vertex IDs, and the precomputed forward
 *          and reverse landmark-distance tables.
 *
 *          The index is constructed through alt_index_build() and released
 *          through alt_index_destroy(). Once construction succeeds, its
 *          contents remain unchanged for the lifetime of the index.
 *
 *          All owned resources are independent of the caller-provided
 *          edge array and landmark configuration. The caller may modify
 *          or release those inputs after construction completes.
 *
 *          A zero-initialized index owns no allocations and supports
 *          cleanup during partially completed construction.
 *
 * @par Invariants
 * - A successfully constructed index contains a valid CSR graph.
 * - The landmark array contains distinct, valid vertex IDs when the
 *   landmark count is positive.
 * - The landmark array is NULL when the landmark count is zero.
 * - The distance tables correspond to the selected landmark array
 *   in its original selection order.
 * - The distance tables use the same vertex count as the CSR graph.
 * - All owned allocations are released by alt_index_destroy().
 */
struct alt_index {
  /** @brief Independently owned CSR representation of the directed graph. */
  alt_graph graph;

  /**
   * @brief Independently allocated array of selected landmark vertex IDs.
   *
   * @details Contains distances.landmark_count entries in selection order.
   *          NULL when no landmarks are selected.
   */
  alt_vertex_id *landmarks;

  /**
   * @brief Owned forward and reverse landmark-distance tables.
   *
   * @details Contains the landmark count, graph vertex count, and
   *          independently allocated row-major distance arrays.
   */
  alt_landmark_distances distances;
};

#endif // ALT_INDEX_H
