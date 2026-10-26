#ifndef ALT_H
#define ALT_H

#include <stddef.h>
#include <stdint.h>

typedef struct alt_index alt_index;

typedef size_t alt_vertex_id;
typedef size_t alt_edge_id;

typedef struct {
  alt_vertex_id source;
  alt_vertex_id target;
  double weight;
} alt_edge;

typedef enum {
  ALT_LANDMARK_EXPLICIT,
  ALT_LANDMARK_RANDOM,
  ALT_LANDMARK_FARTHEST
} alt_landmark_method;

typedef struct {
  alt_landmark_method method;
  size_t count;
  const alt_vertex_id *vertices;
  uint64_t seed;
  alt_vertex_id anchor;
} alt_landmark_options;

typedef enum {
  ALT_OK,
  ALT_NO_PATH,
  ALT_INVALID_ARGUMENT,
  ALT_INVALID_GRAPH,
  ALT_INVALID_CONFIGURATION,
  ALT_OUT_OF_MEMORY,
  ALT_NUMERIC_ERROR
} alt_status;

typedef struct {
  alt_vertex_id *vertices;
  alt_edge_id *edges;
  size_t vertex_count;
  size_t edge_count;
  double total_cost;
} alt_path;

alt_status alt_index_build(size_t vertex_count, const alt_edge *edges,
                           size_t edge_count,
                           const alt_landmark_options *options,
                           alt_index **out_index);

void alt_index_destroy(alt_index *index);

alt_status alt_distance(const alt_index *index, alt_vertex_id start,
                        alt_vertex_id target, double *out_distance);

alt_status alt_shortest_path(const alt_index *index, alt_vertex_id start,
                             alt_vertex_id target, alt_path **out_path);

void alt_path_destroy(alt_path *path);

#endif
