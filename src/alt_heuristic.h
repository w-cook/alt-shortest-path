#ifndef ALT_HEURISTIC_H
#define ALT_HEURISTIC_H

#include "alt/alt.h"
#include "landmark_preprocessing.h"

/**
 * @brief Evaluates the directed ALT heuristic for a vertex and target.
 *
 * @details Computes a nonnegative shortest-path lower-bound estimate
 *          using precomputed landmark distances and the directed
 *          triangle inequality.
 *
 *          For each landmark l, two candidate bounds are evaluated:
 *
 *              d(l, target) - d(l, current)
 *              d(current, l) - d(target, l)
 *
 *          The result is the maximum of zero and all usable bounds
 *          across the selected landmarks.
 *
 *          Each bound is considered independently and only when both
 *          operands are finite. Bounds involving unreachable distances
 *          are skipped without performing infinity subtraction.
 *
 *          When no landmarks are selected, or no usable bound exists,
 *          the heuristic evaluates to zero.
 *
 *          The supplied distance tables must satisfy the invariants
 *          established by successful landmark preprocessing. Both
 *          vertex identifiers must be within the stored vertex count.
 *
 *          The function performs no allocations and does not modify
 *          the precomputed distance tables.
 *
 *          Under exact arithmetic, the heuristic is admissible for
 *          reachable vertex-target pairs. Floating-point rounding
 *          may slightly violate this property for some inputs.
 *
 * @param[in] distances
 *     Pointer to valid precomputed directed landmark-distance tables.
 *
 * @param[in] current_vertex
 *     Vertex for which the heuristic estimate is calculated.
 *
 * @param[in] target_vertex
 *     Destination vertex against which the estimate is calculated.
 *
 * @param[out] out_estimate
 *     Pointer receiving the nonnegative heuristic estimate on success.
 *     The output is not a valid result if the function fails.
 *
 * @retval ALT_OK
 *     Heuristic evaluation completed successfully, including cases
 *     with zero landmarks or no usable bounds.
 * @retval ALT_INVALID_ARGUMENT
 *     A required pointer is NULL or a vertex identifier is outside
 *     the stored graph's valid range.
 */
alt_status alt_heuristic_evaluate(const alt_landmark_distances *distances,
                                  alt_vertex_id current_vertex,
                                  alt_vertex_id target_vertex,
                                  double *out_estimate);

#endif
