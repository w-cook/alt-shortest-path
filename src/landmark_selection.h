#ifndef ALT_LANDMARK_SELECTION_H
#define ALT_LANDMARK_SELECTION_H

#include "alt/alt.h"
#include "graph.h"

/**
 * @brief Selects landmark vertices according to the specified selection method.
 *
 * @details Supports explicit, reproducible random, and farthest-first landmark
 *          selection. All selected landmark IDs must be valid graph vertices
 *          and must be distinct.
 *
 *          The caller retains ownership of the graph and selection options.
 *          On success, the caller takes ownership of the allocated landmark
 *          array and must release it using free().
 *
 *          When the requested landmark count is zero, selection succeeds
 *          without allocating an array, and *out_landmarks remains NULL.
 *          On failure, *out_landmarks is set to NULL.
 *
 * @param[in] graph
 *     Pointer to a valid CSR graph. The graph is not modified.
 *
 * @param[in] options
 *     Pointer to the landmark-selection configuration, specifying the
 *     selection method, count, and method-specific parameters.
 *
 * @param[out] out_landmarks
 *     Pointer receiving an independently allocated array of selected
 *     landmark vertex IDs, or NULL when the count is zero.
 *
 * @retval ALT_OK
 *     Landmark selection completed successfully.
 * @retval ALT_INVALID_ARGUMENT
 *     A required pointer is NULL.
 * @retval ALT_INVALID_CONFIGURATION
 *     The selection method, landmark count, vertex IDs, or other
 *     method-specific configuration is invalid.
 * @retval ALT_OUT_OF_MEMORY
 *     A required allocation failed or its size cannot be represented.
 * @retval ALT_NUMERIC_ERROR
 *     A numerical failure occurred during distance-based selection.
 */
alt_status landmark_select(const alt_graph *graph,
                           const alt_landmark_options *options,
                           alt_vertex_id **out_landmarks);

#endif // ALT_LANDMARK_SELECTION_H
