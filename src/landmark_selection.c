#include "landmark_selection.h"
#include "alt/alt.h"
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
  case ALT_LANDMARK_FARTHEST:
    // Temporary failure status
    return ALT_INVALID_CONFIGURATION;
  }

  // Transfer ownership of the local array to caller
  *out_landmarks = local_landmarks;

  return ALT_OK;
}
