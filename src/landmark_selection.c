#include "landmark_selection.h"
#include "alt/alt.h"
#include <stdint.h>
#include <stdlib.h>

// Selects landmark vertices according to the specified selection method.
alt_status landmark_select(const alt_graph *graph,
                           const alt_landmark_options *options,
                           alt_vertex_id **out_landmarks) {
  // Validate output destination
  if (out_landmarks == NULL) {
    return ALT_INVALID_ARGUMENT;
  }

  // Initialize the output destination
  *out_landmarks = NULL;

  // Validate the remaining pointer arguments
  if (graph == NULL || options == NULL) {
    return ALT_INVALID_ARGUMENT;
  }

  // Validate that the selection method is recognized and the requested count
  // does not exceed the graph's vertex count
  if (options->method < ALT_LANDMARK_EXPLICIT ||
      options->method > ALT_LANDMARK_FARTHEST ||
      options->count > graph->vertex_count) {
    return ALT_INVALID_CONFIGURATION;
  }

  // Zero landmarks must succeed without allocating anything
  if (options->count == 0) {
    return ALT_OK;
  }

  switch (options->method) {
  case ALT_LANDMARK_EXPLICIT:
    // Verify that the vertex array is non-null
    if (options->vertices == NULL) {
      return ALT_INVALID_CONFIGURATION;
    }

    // Validate that every supplied landmark ID is within the graph's vertex
    // range
    for (size_t landmark_id_i = 0; landmark_id_i < options->count;
         landmark_id_i++) {
      if (options->vertices[landmark_id_i] >= graph->vertex_count) {
        return ALT_INVALID_CONFIGURATION;
      }
    }

    // Verify that no two selected landmark ID's are identical
    for (size_t landmark_id_i = 0; landmark_id_i < options->count;
         landmark_id_i++) {
      for (size_t compare_landmark_id_i = 0;
           compare_landmark_id_i < landmark_id_i; compare_landmark_id_i++) {
        if (options->vertices[landmark_id_i] ==
            options->vertices[compare_landmark_id_i]) {
          return ALT_INVALID_CONFIGURATION;
        }
      }
    }

    // Allocate an independently owned array to avoid modifying caller's in case
    // of failure
    if (options->count > SIZE_MAX / sizeof(alt_vertex_id)) {
      return ALT_OUT_OF_MEMORY;
    }

    alt_vertex_id *local_landmarks =
        malloc(sizeof(alt_vertex_id) * options->count);

    if (local_landmarks == NULL) {
      return ALT_OUT_OF_MEMORY;
    }

    // Copy the validated IDs
    for (size_t landmark_id_i = 0; landmark_id_i < options->count;
         landmark_id_i++) {
      local_landmarks[landmark_id_i] = options->vertices[landmark_id_i];
    }

    // Transfer ownership of the local array to caller
    *out_landmarks = local_landmarks;

    break;
  case ALT_LANDMARK_RANDOM:
  case ALT_LANDMARK_FARTHEST:
    // Temporary failure status
    return ALT_INVALID_CONFIGURATION;
  }

  return ALT_OK;
}
