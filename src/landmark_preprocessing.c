#include "landmark_preprocessing.h"
#include "alt/alt.h"
#include "dijkstra.h"
#include "graph.h"
#include <stdint.h>
#include <stdlib.h>

// Precomputes directed shortest-path distances for selected landmarks
alt_status landmark_preprocess(const alt_graph *graph,
                               const alt_vertex_id *landmarks,
                               size_t landmark_count,
                               alt_landmark_distances *out_distances) {
  // Validate arguments
  if (out_distances == NULL || graph == NULL ||
      landmark_count > graph->vertex_count ||
      (landmark_count > 0 && landmarks == NULL)) {
    return ALT_INVALID_ARGUMENT;
  }

  // Reject out-of-range landmark ids defensively
  for (size_t landmark_i = 0; landmark_i < landmark_count; landmark_i++) {
    if (landmarks[landmark_i] >= graph->vertex_count) {
      return ALT_INVALID_ARGUMENT;
    }
  }

  // A zero landmark count does not require any additional work
  if (landmark_count == 0) {
    out_distances->vertex_count = graph->vertex_count;
    return ALT_OK;
  }

  // Compute the number of elements for the distance tables
  if (landmark_count > SIZE_MAX / graph->vertex_count) {
    return ALT_OUT_OF_MEMORY;
  }

  size_t element_count = landmark_count * graph->vertex_count;

  // Create a local instance of alt_landmark_distances to later map to later
  // reassign ownership of in order to prevent modifying the callers structure
  // in case of failure
  alt_landmark_distances local_distances = {0};

  // Allocate the distance tables
  if (element_count > SIZE_MAX / sizeof(double)) {
    return ALT_OUT_OF_MEMORY;
  }

  local_distances.distance_from = malloc(element_count * sizeof(double));

  if (local_distances.distance_from == NULL) {
    return ALT_OUT_OF_MEMORY;
  }

  local_distances.distance_to = malloc(element_count * sizeof(double));

  if (local_distances.distance_to == NULL) {
    landmark_distances_destroy(&local_distances);
    return ALT_OUT_OF_MEMORY;
  }

  // Finish initializing the local structure
  local_distances.landmark_count = landmark_count;
  local_distances.vertex_count = graph->vertex_count;

  // Create a local reversed graph
  alt_graph graph_reverse = {0};
  alt_status result = graph_build_reverse(graph, &graph_reverse);

  // Propagate result on failure
  if (result != ALT_OK) {
    landmark_distances_destroy(&local_distances);
    return result;
  }

  // Calculate distances for all landmarks
  for (size_t landmark_i = 0; landmark_i < landmark_count; landmark_i++) {
    alt_vertex_id landmark_vertex = landmarks[landmark_i];
    size_t landmark_row_offset = landmark_i * graph->vertex_count;

    // Calculate distances from based on original graph
    result =
        dijkstra_all(graph, landmark_vertex,
                     &(local_distances.distance_from[landmark_row_offset]));

    // Propagate result on failure
    if (result != ALT_OK) {
      landmark_distances_destroy(&local_distances);
      graph_destroy(&graph_reverse);
      return result;
    }

    // Calculate distances to based on reversed graph
    result = dijkstra_all(&graph_reverse, landmark_vertex,
                          &(local_distances.distance_to[landmark_row_offset]));

    // Propagate result on failure
    if (result != ALT_OK) {
      landmark_distances_destroy(&local_distances);
      graph_destroy(&graph_reverse);
      return result;
    }
  }

  // Transfer ownership of the local distances structure to the caller
  *out_distances = local_distances;

  // Cleanup temporary allocations
  graph_destroy(&graph_reverse);

  return ALT_OK;
}

// Releases all memory owned by precomputed landmark-distance tables
void landmark_distances_destroy(alt_landmark_distances *distances) {
  // Handle null argument
  if (distances == NULL) {
    return;
  }

  // Release both arrays
  free(distances->distance_from);
  free(distances->distance_to);

  // Reset the structure
  distances->distance_from = NULL;
  distances->distance_to = NULL;
  distances->vertex_count = 0;
  distances->landmark_count = 0;
}
