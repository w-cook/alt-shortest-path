#ifndef ALT_DIJKSTRA_H
#define ALT_DIJKSTRA_H

#include "alt/alt.h"
#include "graph.h"

/**
 * @brief Calculates shortest distances from a source vertex to every vertex
 *        in a directed, weighted graph.
 *
 * @details Uses Dijkstra's algorithm to calculate the shortest distance from
 *          the specified source vertex to every vertex in the graph.
 *          Unreachable vertices are assigned positive infinity.
 *
 *          The caller retains ownership of the graph and output array.
 *          The output array must contain at least graph->vertex_count elements.
 *          Its contents must not be relied upon if the function fails.
 *
 * @param[in] graph
 *     Pointer to a valid CSR graph. The graph is not modified.
 *
 * @param[in] source_vertex_id
 *     Identifier of the source vertex. Must be less than graph->vertex_count.
 *
 * @param[out] out_distances
 *     Caller-allocated array receiving the shortest distance to each vertex,
 *     indexed by vertex ID. Unreachable vertices receive positive infinity.
 *
 * @retval ALT_OK
 *     All shortest distances were calculated successfully.
 * @retval ALT_INVALID_ARGUMENT
 *     A required pointer is null or the source vertex ID is invalid.
 * @retval ALT_OUT_OF_MEMORY
 *     A required internal memory allocation failed.
 * @retval ALT_NUMERIC_ERROR
 *     A numerical operation produced an unsupported or invalid result.
 *     This includes when a distance accumulation produces a nonfinite
 *     result, such as overflow to infinity, and when adding a strictly
 *     positive edge weight leaves the accumulated distance unchanged
 *     because of floating-point rounding.
 */
alt_status dijkstra_all(const alt_graph *graph, alt_vertex_id source_vertex_id,
                        double *out_distances);

#endif // ALT_DIJKSTRA_H
