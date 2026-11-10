#include "alt_index.h"
#include "alt/alt.h"
#include "graph.h"
#include "landmark_preprocessing.h"
#include "landmark_selection.h"
#include <stdlib.h>

// Constructs a reusable ALT shortest-path index from a directed, weighted graph
alt_status alt_index_build(size_t vertex_count, const alt_edge *edges,
                           size_t edge_count,
                           const alt_landmark_options *options,
                           alt_index **out_index) {
  // Validate and initialize the output
  if (out_index == NULL) {
    return ALT_INVALID_ARGUMENT;
  }

  *out_index = NULL;

  // Validate options
  if (options == NULL) {
    return ALT_INVALID_ARGUMENT;
  }

  // Allocate a zero-initialized index, retaining ownership until construction
  // succeeds to avoid modifying the output in case of failure
  alt_index *local_index = calloc(1, sizeof(alt_index));

  if (local_index == NULL) {
    return ALT_OUT_OF_MEMORY;
  }

  // Construct the locally owned graph and propagate any failure
  alt_status result =
      graph_build(vertex_count, edges, edge_count, &local_index->graph);

  if (result != ALT_OK) {
    alt_index_destroy(local_index);
    return result;
  }

  // Select landmarks and propagate any failure
  result =
      landmark_select(&local_index->graph, options, &local_index->landmarks);

  if (result != ALT_OK) {
    alt_index_destroy(local_index);
    return result;
  }

  // Preprocess the selected landmarks and propagate any failure
  result = landmark_preprocess(&local_index->graph, local_index->landmarks,
                               options->count, &local_index->distances);

  if (result != ALT_OK) {
    alt_index_destroy(local_index);
    return result;
  }

  // Assign ownership of the locally constructed alt_index to the caller and
  // return success
  *out_index = local_index;

  return ALT_OK;
}

// Destroys an ALT index and releases all resources it owns
void alt_index_destroy(alt_index *index) {
  // Safely handle null argument
  if (index == NULL) {
    return;
  }

  // Release allocations
  graph_destroy(&index->graph);
  free(index->landmarks);
  landmark_distances_destroy(&index->distances);
  free(index);
}
