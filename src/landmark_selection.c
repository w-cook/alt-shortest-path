#include "landmark_selection.h"
#include "alt/alt.h"
#include "dijkstra.h"
#include "graph.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

// Private deterministic 64-bit pseudorandom generator implementation of
// SplitMix64
// state != NULL is a precondition expected to be satisfied by the caller
static uint64_t next_random_64(uint64_t *state) {
  const uint64_t increment = UINT64_C(0x9E3779B97F4A7C15);
  const uint64_t multiplier_one = UINT64_C(0xBF58476D1CE4E5B9);
  const uint64_t multiplier_two = UINT64_C(0x94D049BB133111EB);

  // Step 1: Advance the persistent generator state
  *state = *state + increment;

  // Step 2: Copy the updated state to a working variable
  uint64_t working_val = *state;

  // Step 3: First mixing operation
  working_val = (working_val ^ (working_val >> 30)) * multiplier_one;

  // Step 4: Second mixing operation
  working_val = (working_val ^ (working_val >> 27)) * multiplier_two;

  // Step 5: Final mixing operation
  working_val = working_val ^ (working_val >> 31);

  return working_val;
}

// Private unbiased bounded random number helper
// state != NULL is a precondition expected to be satisfied by the caller.
// bound > 0 is a precondition expected to be satisfied by the caller.
// Assumes size_t is no wider than 64 bits
static size_t random_below(uint64_t *state, size_t bound) {
  // Convert bound to an unsigned 64-bit integer
  uint64_t bound_uint64 = (uint64_t)bound;

  // Calculate the rejection threshold
  uint64_t threshold = (-bound_uint64) % bound_uint64;

  // Declare a value to store result from SplitMix64
  uint64_t generated_val;

  // Generate values until one falls outside the rejection range
  do {
    generated_val = next_random_64(state);
  } while (generated_val < threshold);

  // Map the accepted sample into [0, bound) and return it as size_t
  return (size_t)(generated_val % bound_uint64);
}

// Constructs an independently owned undirected projection of a directed graph.
// Each original edge contributes two opposite-direction edges with equal
// weight. The output graph must be zero-initialized and is unchanged on failure
static alt_status build_undirected_projection(const alt_graph *graph,
                                              alt_graph *out_graph) {
  // Verify the arguments
  if (graph == NULL || out_graph == NULL || graph == out_graph ||
      graph->edge_starts == NULL ||
      (graph->edge_count > 0 &&
       (graph->targets == NULL || graph->weights == NULL ||
        graph->original_edge_ids == NULL))) {
    return ALT_INVALID_ARGUMENT;
  }

  // Determine the required number of projected edges
  if (graph->edge_count > SIZE_MAX / 2) {
    return ALT_OUT_OF_MEMORY;
  }

  size_t required_edge_count = graph->edge_count * 2;

  // Allocate a temporary array of alt_edge values
  if (required_edge_count > SIZE_MAX / sizeof(alt_edge)) {
    return ALT_OUT_OF_MEMORY;
  }

  alt_edge *edges = required_edge_count == 0
                        ? NULL
                        : malloc(required_edge_count * sizeof(alt_edge));

  if (edges == NULL && required_edge_count != 0) {
    return ALT_OUT_OF_MEMORY;
  }

  // Create duplicates of every edge with the source and destination reversed
  for (size_t edge_starts_i = 0; edge_starts_i < graph->vertex_count;
       edge_starts_i++) {
    alt_vertex_id original_source = edge_starts_i;

    for (size_t edge_i = graph->edge_starts[edge_starts_i];
         edge_i < graph->edge_starts[edge_starts_i + 1]; edge_i++) {
      alt_vertex_id original_target = graph->targets[edge_i];
      double edge_weight = graph->weights[edge_i];

      edges[edge_i] = (alt_edge){original_source, original_target, edge_weight};
      edges[required_edge_count - 1 - edge_i] =
          (alt_edge){original_target, original_source, edge_weight};
    }
  }

  // Create a zero-initialized local graph to temporarily store the created
  // graph in to avoid modifying the caller's in case of failure, and build the
  // undirected graph
  alt_graph local_graph = {0};

  alt_status result = graph_build(graph->vertex_count, edges,
                                  required_edge_count, &local_graph);

  // Cleanup allocations and propagate result on failure
  free(edges);

  if (result != ALT_OK) {
    return result;
  }

  // Transfer ownership of the local graph to caller and return success
  *out_graph = local_graph;

  return ALT_OK;
}

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

  // Declare an independently owned array for output to avoid modifying caller's
  // in case of failure
  alt_vertex_id *local_landmarks = NULL;

  switch (options->method) {
  case ALT_LANDMARK_EXPLICIT: {
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

    // Allocate the local output array
    if (options->count > SIZE_MAX / sizeof(alt_vertex_id)) {
      return ALT_OUT_OF_MEMORY;
    }

    local_landmarks = malloc(sizeof(alt_vertex_id) * options->count);

    if (local_landmarks == NULL) {
      return ALT_OUT_OF_MEMORY;
    }

    // Copy the validated IDs
    for (size_t landmark_id_i = 0; landmark_id_i < options->count;
         landmark_id_i++) {
      local_landmarks[landmark_id_i] = options->vertices[landmark_id_i];
    }

    break;
  }
  case ALT_LANDMARK_RANDOM: {
    // Create a candidate array containing every vertex in order
    if (graph->vertex_count > SIZE_MAX / sizeof(alt_vertex_id)) {
      return ALT_OUT_OF_MEMORY;
    }

    alt_vertex_id *candidates =
        malloc(sizeof(alt_vertex_id) * graph->vertex_count);

    if (candidates == NULL) {
      return ALT_OUT_OF_MEMORY;
    }

    for (size_t i = 0; i < graph->vertex_count; i++) {
      candidates[i] = i;
    }

    // The output allocation is size-safe because count does not exceed
    // vertex_count, which was checked above
    local_landmarks = malloc(sizeof(alt_vertex_id) * options->count);

    if (local_landmarks == NULL) {
      free(candidates);
      return ALT_OUT_OF_MEMORY;
    }

    // Initialize the generator state from options->seed
    uint64_t generator_state = options->seed;

    // Partial Fisher-Yates shuffle: select landmarks without replacement.
    // At iteration i, choose uniformly from the remaining candidates
    // in [i, vertex_count), move the chosen vertex into position i,
    // and copy it to the output array
    for (size_t i = 0; i < options->count; i++) {
      size_t offset = random_below(&generator_state, graph->vertex_count - i);

      size_t j = i + offset;

      alt_vertex_id temp_vertex = candidates[j];
      candidates[j] = candidates[i];
      candidates[i] = temp_vertex;

      local_landmarks[i] = candidates[i];
    }

    // Release the candidate array
    free(candidates);

    break;
  }
  case ALT_LANDMARK_FARTHEST: {
    // Validate the provided anchor
    if (options->anchor >= graph->vertex_count) {
      return ALT_INVALID_CONFIGURATION;
    }

    // Build an undirected projection of the graph
    alt_graph undirected_graph = {0};

    alt_status result = build_undirected_projection(graph, &undirected_graph);

    // Check result and propagate failure
    if (result != ALT_OK) {
      return result;
    }

    // Allocate a temporary distance array
    if (undirected_graph.vertex_count > SIZE_MAX / sizeof(double)) {
      graph_destroy(&undirected_graph);
      return ALT_OUT_OF_MEMORY;
    }

    double *distances = malloc(sizeof(double) * undirected_graph.vertex_count);

    if (distances == NULL) {
      graph_destroy(&undirected_graph);
      return ALT_OUT_OF_MEMORY;
    }

    // Run Dijkstra on the projection using the anchor as the source
    result = dijkstra_all(&undirected_graph, options->anchor, distances);

    // Cleanup allocations and propagate error on failure
    if (result != ALT_OK) {
      free(distances);
      graph_destroy(&undirected_graph);
      return result;
    }

    // Allocate the local output array
    if (options->count > SIZE_MAX / sizeof(alt_vertex_id)) {
      free(distances);
      graph_destroy(&undirected_graph);
      return ALT_OUT_OF_MEMORY;
    }

    local_landmarks = malloc(sizeof(alt_vertex_id) * options->count);

    if (local_landmarks == NULL) {
      free(distances);
      graph_destroy(&undirected_graph);
      return ALT_OUT_OF_MEMORY;
    }

    // Find the farthest vertex with a finite distance from the anchor
    double farthest_distance = 0;
    alt_vertex_id farthest_vertex = options->anchor;

    for (size_t vertex_i = 0; vertex_i < undirected_graph.vertex_count;
         vertex_i++) {
      if (isfinite(distances[vertex_i]) &&
          distances[vertex_i] > farthest_distance) {
        farthest_distance = distances[vertex_i];
        farthest_vertex = vertex_i;
      }
    }

    // Assign the farthest vertex with a finite distance as the first landmark
    local_landmarks[0] = farthest_vertex;

    // The following allocations are unnessecary if the landmark count has been
    // reached
    double *distances_two = NULL;
    bool *vertex_was_selected = NULL;

    if (options->count > 1) {
      // Allocate another temporary array of distances and an array of boolean
      // flags to track which vertices have already been selected. The distance
      // array can be assumed to be size-safe because the same calculation was
      // checked previously
      distances_two = malloc(sizeof(double) * undirected_graph.vertex_count);

      if (distances_two == NULL) {
        free(distances);
        free(local_landmarks);
        graph_destroy(&undirected_graph);
        return ALT_OUT_OF_MEMORY;
      }

      if (undirected_graph.vertex_count > SIZE_MAX / sizeof(bool)) {
        free(distances);
        free(distances_two);
        free(local_landmarks);
        graph_destroy(&undirected_graph);
        return ALT_OUT_OF_MEMORY;
      }

      vertex_was_selected =
          malloc(sizeof(bool) * undirected_graph.vertex_count);

      if (vertex_was_selected == NULL) {
        free(distances);
        free(distances_two);
        free(local_landmarks);
        graph_destroy(&undirected_graph);
        return ALT_OUT_OF_MEMORY;
      }

      // Initialize the boolean flag array
      for (size_t vertex_i = 0; vertex_i < undirected_graph.vertex_count;
           vertex_i++) {
        vertex_was_selected[vertex_i] = false;
      }
      vertex_was_selected[local_landmarks[0]] = true;

      // Run Dijstra on the undirected projection from the first selected
      // landmark
      result =
          dijkstra_all(&undirected_graph, local_landmarks[0], distances_two);

      // Cleanup allocations and propagate error on failure
      if (result != ALT_OK) {
        free(distances);
        free(distances_two);
        free(vertex_was_selected);
        free(local_landmarks);
        graph_destroy(&undirected_graph);
        return result;
      }

      // Determine the remaining landmarks
      for (size_t landmark_i = 1; landmark_i < options->count; landmark_i++) {
        farthest_distance = -INFINITY;
        farthest_vertex = SIZE_MAX;

        // Scan the vertices in ascending id order, considering only vertices
        // that haven't already been selected
        for (size_t vertex_i = 0; vertex_i < undirected_graph.vertex_count;
             vertex_i++) {
          if (!vertex_was_selected[vertex_i] &&
              distances_two[vertex_i] > farthest_distance) {
            farthest_distance = distances_two[vertex_i];
            farthest_vertex = vertex_i;
          }
        }

        // Record the selection
        local_landmarks[landmark_i] = farthest_vertex;
        vertex_was_selected[farthest_vertex] = true;

        // Update nearest-landmark distances if necessary
        if (landmark_i < options->count - 1) {
          result = dijkstra_all(&undirected_graph, farthest_vertex, distances);

          // Cleanup allocations and propagate error on failure
          if (result != ALT_OK) {
            free(distances);
            free(distances_two);
            free(vertex_was_selected);
            free(local_landmarks);
            graph_destroy(&undirected_graph);
            return result;
          }

          // distances_two should always represent the distance to the nearest
          // selected landmark
          for (size_t vertex_i = 0; vertex_i < undirected_graph.vertex_count;
               vertex_i++) {
            if (distances[vertex_i] < distances_two[vertex_i]) {
              distances_two[vertex_i] = distances[vertex_i];
            }
          }
        }
      }
    }

    // Release temporary allocations
    free(distances);
    free(distances_two);
    free(vertex_was_selected);
    graph_destroy(&undirected_graph);
  }
  }

  // Transfer ownership of the local array to caller
  *out_landmarks = local_landmarks;

  return ALT_OK;
}
