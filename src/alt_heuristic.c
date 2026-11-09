#include "alt_heuristic.h"
#include <math.h>

// Evaluates the directed ALT heuristic for a vertex and target.
alt_status alt_heuristic_evaluate(const alt_landmark_distances *distances,
                                  alt_vertex_id current_vertex,
                                  alt_vertex_id target_vertex,
                                  double *out_estimate) {
  // Validate the arguments
  if (distances == NULL || out_estimate == NULL ||
      current_vertex >= distances->vertex_count ||
      target_vertex >= distances->vertex_count) {
    return ALT_INVALID_ARGUMENT;
  }

  // Zero landmarks or same vertex does not need any calculations
  if (distances->landmark_count == 0 || current_vertex == target_vertex) {
    *out_estimate = 0.0;
    return ALT_OK;
  }

  // Determine the maximum nonnegative estimate using the formula:
  // Forward: d(l,t) - d(l,v)
  // Reverse: d(v,l) - d(t,l)
  double maximum_nonnegative_estimate = 0.0;

  for (size_t landmark_i = 0; landmark_i < distances->landmark_count;
       landmark_i++) {
    size_t landmark_row_offset = landmark_i * distances->vertex_count;
    double current_distance_from_landmark =
        distances->distance_from[current_vertex + landmark_row_offset];
    double target_distance_from_landmark =
        distances->distance_from[target_vertex + landmark_row_offset];
    double current_distance_to_landmark =
        distances->distance_to[current_vertex + landmark_row_offset];
    double target_distance_to_landmark =
        distances->distance_to[target_vertex + landmark_row_offset];

    // Only evaluate finite values for both operands as possible candidates
    if (isfinite(current_distance_from_landmark) &&
        isfinite(target_distance_from_landmark) &&
        target_distance_from_landmark - current_distance_from_landmark >
            maximum_nonnegative_estimate) {
      maximum_nonnegative_estimate =
          target_distance_from_landmark - current_distance_from_landmark;
    }

    if (isfinite(current_distance_to_landmark) &&
        isfinite(target_distance_to_landmark) &&
        current_distance_to_landmark - target_distance_to_landmark >
            maximum_nonnegative_estimate) {
      maximum_nonnegative_estimate =
          current_distance_to_landmark - target_distance_to_landmark;
    }
  }

  // Assign the determined value to the output and return success
  *out_estimate = maximum_nonnegative_estimate;

  return ALT_OK;
}
