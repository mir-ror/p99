
#ifndef   	ORWL_GRAPH_H_
# define   	ORWL_GRAPH_H_

#include "orwl_int.h"

P99_DECLARE_STRUCT(orwl_neighbor);

struct orwl_neighbor {
  size_t id;
  size_t nb_locations;
  size_t * locations;
};

struct orwl_neighbor* orwl_neighbor_init(struct orwl_neighbor *neighbor,
					 size_t id,
					 size_t nb_locations) {
  if (neighbor) {
    *neighbor = P99_LVAL(struct orwl_neighbor,
			 .id = id,
			 .nb_locations = nb_locations,
			 .locations = NULL,
			 );
    if (nb_locations > 0)
      neighbor->locations = size_t_vnew(nb_locations);
  }
  return neighbor;
}

P99_PROTOTYPE(struct orwl_neighbor*, orwl_neighbor_init, struct orwl_neighbor*, size_t, size_t);
#define orwl_neighbor_init(...) P99_CALL_DEFARG(orwl_neighbor_init, 3, __VA_ARGS__)
P99_DECLARE_DEFARG(orwl_neighbor_init, , P99_0(size_t), P99_0(size_t));
P99_DEFINE_DEFARG(orwl_neighbor_init, , P99_0(size_t), P99_0(size_t));


void orwl_neighbor_destroy(struct orwl_neighbor *neighbor) {
  size_t_vdelete(neighbor->locations);
}

DECLARE_NEW_DELETE(orwl_neighbor);
DEFINE_NEW_DELETE(orwl_neighbor);


P99_DECLARE_STRUCT(orwl_vertice);

struct orwl_vertice {
  size_t id;
  size_t color;
  size_t nb_neighbors;
  struct orwl_neighbor * neighbors;
};

struct orwl_vertice* orwl_vertice_init(struct orwl_vertice *vertice,
				       size_t id,
				       size_t color,
				       size_t nb_neighbors) {
  if (vertice) {
    *vertice = P99_LVAL(struct orwl_vertice,
  			.id = id,
  			.color = color,
  			.nb_neighbors = nb_neighbors,
  			.neighbors = NULL,
  			);
    if (nb_neighbors > 0)
      vertice->neighbors = orwl_neighbor_vnew(nb_neighbors);
  }
 
  return vertice;
}

P99_PROTOTYPE(struct orwl_vertice*, orwl_vertice_init, struct orwl_vertice*, size_t, size_t, size_t);
#define orwl_vertice_init(...) P99_CALL_DEFARG(orwl_vertice_init, 4, __VA_ARGS__)
P99_DECLARE_DEFARG(orwl_vertice_init, , P99_0(size_t), P99_0(size_t), P99_0(size_t));
P99_DEFINE_DEFARG(orwl_vertice_init, , P99_0(size_t), P99_0(size_t), P99_0(size_t));


void orwl_vertice_destroy(struct orwl_vertice *vertice) {
  orwl_neighbor_vdelete(vertice->neighbors);
}

DECLARE_NEW_DELETE(orwl_vertice);
DEFINE_NEW_DELETE(orwl_vertice);


P99_DECLARE_STRUCT(orwl_graph);

struct orwl_graph {
  size_t nb_vertices;
  struct orwl_vertice *vertices;
};

struct orwl_graph * orwl_graph_init(struct orwl_graph * graph, size_t nb_vertices) {
  graph->nb_vertices = nb_vertices;
  graph->vertices = orwl_vertice_vnew(nb_vertices);
  return graph;
}

P99_PROTOTYPE(struct orwl_graph*, orwl_graph_init, struct orwl_graph *, size_t);
#define orwl_graph_init(...) P99_CALL_DEFARG(orwl_graph_init, 2, __VA_ARGS__)
P99_DECLARE_DEFARG(orwl_graph_init, , P99_0(size_t));
P99_DEFINE_DEFARG(orwl_graph_init, , P99_0(size_t));

void orwl_graph_destroy(struct orwl_graph *graph) {
  orwl_vertice_vdelete(graph->vertices);
}

DECLARE_NEW_DELETE(orwl_graph);
DEFINE_NEW_DELETE(orwl_graph);



struct orwl_graph * orwl_graph_read(struct orwl_graph * graph, char const* file);



#endif