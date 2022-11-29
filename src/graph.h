/*
 *
 * Created by:   github.com/johnstef99
 * Last updated: 2022-11-18
 *
 */

#ifndef GRAPH_H
#define GRAPH_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <cilk/cilk.h>

#include "csx.h"

void size_t_zero(void *view); 
void size_t_add(void *left, void *right);
void size_t_destroy(void *view);

struct Graph {
  /* graph's number of vertices */
  size_t v;

  /* graph's number of edges */
  size_t e;

  /* graph's representation in CSC*/
  csx in;

  /* mask to keep which vertices have been removed */
  bool *removed;

  /* an array of the scc_id for each vertice (vertices with the same scc_id
   * belong to the same SCC)*/
  size_t *scc_id;

  /* number to keep track of how many vertices have been trimmed (it's a
   * cilk_reducer)*/
  size_t cilk_reducer(size_t_zero, size_t_add)* n_trimmed;
};

typedef struct Graph *graph;

graph graph_new_from_csc(csx csc);

void graph_free(graph g);

void graph_trim(graph g);

void graph_bfs(graph g, size_t entry, size_t *colors);

void graph_colorSCC(graph g);

#endif
