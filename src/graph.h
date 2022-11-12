/*
 *
 * Created by:   github.com/johnstef99
 * Last updated: 2022-11-12
 *
 */

#ifndef GRAPH_H
#define GRAPH_H

#include "node.h"

struct Graph {
  /* num of vertices */
  int v;
  node *nodes;
};

typedef struct Graph *graph;

graph graph_new(int vertices);

void graph_free(graph *g);

void graph_add_edge(graph *g, int s, int d);

graph graph_T(graph g);

void graph_dfs(graph g, int s, bool visitedV[]);

void graph_fill_order(graph g, int s, bool visitedV[], node *stack);

void graph_print_scc(graph g);

#endif
