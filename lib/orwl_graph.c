#include "orwl_graph.h"
#include "p99_c99_default.h"
#include "p99_str.h"

#define MAX_LINE_SIZE 2048

unsigned strcountchr(char *str, int chr) {
  unsigned count = 0;
  char *ptr = str;
  while (ptr != NULL) {
    ptr = strchr(ptr, chr);
    if (ptr != NULL) {
      count++;
      ptr = ptr + 1;
    }
  }
  return count;
}

unsigned strsplit(char *str, char str_array[][MAX_LINE_SIZE], char *delim) {
  unsigned count = 0;
  char *token;
  char *tmp = strdup(str);
  if (strcountchr(tmp, (int)*delim) > 0) {
    token = strtok(tmp, delim);
    while (token != NULL) {
      snprintf(str_array[count], MAX_LINE_SIZE, "%s", token);
      token = strtok(NULL, delim);
      count++;
    }
  }
  free(tmp);
  return count;
}

struct orwl_neighbor * orwl_graph_find_neighbor(struct orwl_vertice *vertice, size_t id) {
  if (vertice != NULL)
    for (size_t i ; i < vertice->nb_neighbors ; i++)
      if (vertice->neighbors[i].id == id) return &vertice->neighbors[i];
  return NULL;
}

struct orwl_neighbor * orwl_graph_insert_neighbor(struct orwl_vertice * vertice,
						  size_t id,
						  size_t nb_locations) {
  if ((vertice != NULL) && orwl_graph_find_neighbor(vertice, id) == NULL) {
    size_t i = 0;
    while (vertice->neighbors[i].id != 0) i++;
    orwl_neighbor_init(&vertice->neighbors[i], id, nb_locations);
    return &vertice->neighbors[i];
  }
  return NULL;
}

struct orwl_vertice * orwl_graph_find_vertice(struct orwl_graph * graph, size_t id) {
  if (graph != NULL)
    for (size_t i ; i < graph->nb_vertices ; i++)
      if (graph->vertices[i].id == id) return &graph->vertices[i];
  return NULL;
}

struct orwl_vertice * orwl_graph_insert_vertice(struct orwl_graph * graph,
						size_t id,
						size_t color,
						size_t nb_neighbors) {
  if ((graph != NULL) && orwl_graph_find_vertice(graph, id) == NULL) {
    size_t i = 0;
    while (graph->vertices[i].id != 0) i++;
    orwl_vertice_init(&graph->vertices[i], id, color, nb_neighbors);
    return &graph->vertices[i];
  }
  return NULL;
}

unsigned orwl_graph_extract_line(struct orwl_graph * graph, char * str) {
  /*
    A line pattern is as follow:
    vertice_nb,vertice_color:neighbor-loc(,loc)+(;neighbor-loc(,loc)+)+
   */
  char split_l1[2][MAX_LINE_SIZE] = {0};
  if (strsplit(str, split_l1, ":") != 2)
    return 1;
  char split_l11[2][MAX_LINE_SIZE] = {0};
  if (strsplit(split_l1[0], split_l11, ";") != 2)
    return 1;
  size_t id = strtouz(split_l11[0]);
  if (id < 1)
    return 1;
  size_t color = strtouz(split_l11[1]);
  const size_t neighbors = strcountchr(split_l1[1], ';') + 1;
  struct orwl_vertice * pvertice = orwl_graph_insert_vertice(graph, id, color, neighbors);
  char split_l12[neighbors][MAX_LINE_SIZE];
  if (strsplit(split_l1[1], split_l12, ";") != neighbors)
    return 1;
  for (size_t i = 0 ; i < neighbors ; i++) {
    char split_l121[2][MAX_LINE_SIZE] = {0};
    if (strsplit(split_l12[i], split_l121, "-") != 2)
      return 1;
    size_t id_neighbor = strtouz(split_l121[0]);
    const size_t nb_locations = strcountchr(split_l121[1], ',') + 1;
    struct orwl_neighbor * pneighbor = orwl_graph_insert_neighbor(pvertice, id_neighbor, nb_locations);
    char split_l122[nb_locations][MAX_LINE_SIZE];
    if (strsplit(split_l121[1], split_l122, ",") != nb_locations)
      return 1;
    for (size_t j = 0 ; j < nb_locations ; j++)
      pneighbor->locations[j] = strtouz(split_l122[j]);
  }
  return 0;
}

struct orwl_graph * orwl_graph_read(struct orwl_graph * graph, char const* file) {
    FILE*volatile f = fopen(file, "r");
    if (f == NULL) {
      P99_HANDLE_ERRNO {
      P99_XDEFAULT : {
	  perror("error when opening graph file");
	  return NULL;
	}
      }
    }
    P99_UNWIND_PROTECT {
      char msg[6] = {0};
      if (!fgets(msg, 6, f)) {
	P99_HANDLE_ERRNO {
	P99_XDEFAULT : {
	    perror("error when reading the number of vertices in graph file");
	    P99_UNWIND_RETURN NULL;
	  }
	}
      }
      size_t nb_vertices = str2uz(msg);
      graph = P99_NEW(orwl_graph, nb_vertices);
      P99_UNWIND_PROTECT {
	for (size_t i = 0; i < nb_vertices; i++) {
	  char msg[MAX_LINE_SIZE] = {0};
	  if (!fgets(msg, MAX_LINE_SIZE, f)) {
	    P99_HANDLE_ERRNO {
	    P99_XDEFAULT : {
		perror("error when reading the number of vertices in graph file");
		P99_UNWIND_RETURN NULL;
	      }
	    }
	  }
	  if (orwl_graph_extract_line(graph, msg) != 0) {
	    printf("error while extracting a line in the graph file");
	    P99_UNWIND_RETURN NULL;
	  }
	    
	}
      P99_PROTECT:
	orwl_graph_delete(graph);
      }
 
    P99_PROTECT:
      fclose(f);
    }
    return graph;
}