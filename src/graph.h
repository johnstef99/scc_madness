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

#include "fifo.h"
#include "node.h"

struct Graph {
  size_t v;
  node *edges;
  node *r_edges;
  bool *removed;
  size_t *scc_id;
  size_t n_trimmed;
};

typedef struct Graph *graph;

graph graph_new(size_t v);

void graph_trim(graph g);

uint8_t graph_add_edge(graph g, size_t s, size_t d);

node graph_bfs(graph g, size_t entry, size_t *colors);

void graph_colorSCC(graph g);

#endif

