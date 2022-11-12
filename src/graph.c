/*
 *
 * Created by:   github.com/johnstef99
 * Last updated: 2022-11-12
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "graph.h"
#include "node.h"

graph graph_new(int v) {
  graph g = malloc(v * sizeof(node));
  if (!g) {
    fprintf(stderr, "Graph - Memory allocation failed!\n");
  }

  g->v = v;
  g->nodes = malloc(v * sizeof(node));
  if (!g->nodes) {
    fprintf(stderr, "Graph.nodes - Memory allocation failed!\n");
  }

  return g;
}

void graph_free(graph *g) {
  for (int i = 0; i < (*g)->v; i++) {
    node_free(&(*g)->nodes[i]);
  }
  free((*g)->nodes);
  free(*g);
}

void graph_add_edge(graph *g, int s, int d) { node_push(&(*g)->nodes[s], d); }

graph graph_T(graph g) {
  graph t = graph_new(g->v);
  for (int i = 0; i < g->v; i++) {
    while (!node_is_empty(g->nodes[i])) {
      graph_add_edge(&t, node_pop(&g->nodes[i]), i);
    }
  }
  return t;
}

void graph_dfs(graph g, int s, bool visitedV[]) {
  visitedV[s] = true;
  printf("%d ", s);

  node n = g->nodes[s];
  while (!node_is_empty(n)) {
    if (!visitedV[n->data])
      graph_dfs(g, n->data, visitedV);
    n = n->next;
  }
}

void graph_fill_order(graph g, int s, bool visitedV[], node *stack) {
  visitedV[s] = true;

  node n = g->nodes[s];
  while (!node_is_empty(n)) {
    if (!visitedV[n->data])
      graph_fill_order(g, n->data, visitedV, stack);
    n = n->next;
  }

  node_push(stack, s);
}

void graph_print_scc(graph g) {
  node stack = malloc(sizeof(node));

  bool *visitedV = malloc(g->v * sizeof(bool));
  for (int i = 0; i < g->v; i++)
    visitedV[i] = false;

  for (int i = 0; i < g->v; i++)
    if (!visitedV[i])
      graph_fill_order(g, i, visitedV, &stack);

  graph g_T = graph_T(g);

  for (int i = 0; i < g->v; i++)
    visitedV[i] = false;

  while (!node_is_empty(stack)) {
    int s = node_pop(&stack);

    if (!visitedV[s]) {
      graph_dfs(g_T, s, visitedV);
      puts("");
    }
  }
  free(visitedV);
}
