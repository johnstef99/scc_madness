/*
 *
 * Created by:   github.com/johnstef99
 * Last updated: 2022-11-18
 *
 */

#include "graph.h"
#include "fifo.h"
#include "node.h"

#include <cilk/cilk.h>

graph graph_new(size_t v) {
  graph g = malloc(sizeof(struct Graph));
  if (!g) {
    return NULL;
  }
  g->v = v;
  g->edges = malloc(v * sizeof(struct Node));
  g->r_edges = malloc(v * sizeof(struct Node));
  g->removed = malloc(v * sizeof(bool));
  g->scc_id = malloc(v * sizeof(size_t));
  g->n_trimmed = 0;
  for (size_t i = 0; i < v; i++) {
    g->scc_id[i] = i;
    g->removed[i] = false;
  }
  return g;
}

uint8_t graph_add_edge(graph g, size_t s, size_t d) {
  node_push(&(g->edges[s]), &d, sizeof(size_t));
  return node_push(&(g->r_edges[d]), &s, sizeof(size_t));
}

void graph_trim(graph g) {
  for (size_t v = 0; v < g->v; v++) {
    bool zero_in =
        g->r_edges[v] == NULL ||
        (node_peek_int(g->r_edges[v]) == v && g->r_edges[v]->next == NULL);
    bool zero_out = g->edges[v] == NULL || (node_peek_int(g->edges[v]) == v &&
                                            g->edges[v]->next == NULL);
    if (zero_in || zero_out) {
      g->removed[v] = true;
      g->n_trimmed++;
    }
  }
}

node graph_bfs(graph g, size_t entry, size_t *colors) {
  node scc = node_new(&entry, sizeof(size_t));

  g->removed[entry] = true;

  size_t *vi = malloc(sizeof(size_t));
  if (!vi) {
    fprintf(stderr, "Memory allocation failed!\n");
  }

  node n = g->r_edges[entry];
  fifo q = fifo_new();
  while (n) {
    node_peek(n, vi, sizeof(size_t));
    fifo_enqueue(q, vi, sizeof(size_t));
    n = n->next;
  }

  while (!fifo_is_empty(q)) {
    fifo_dequeue(q, vi, sizeof(size_t));
    if (!g->removed[*vi] && colors[*vi] == entry) {
      g->removed[*vi] = true;
      node_push(&scc, vi, sizeof(size_t));
      n = g->r_edges[*vi];
      while (n) {
        node_peek(n, vi, sizeof(size_t));
        fifo_enqueue(q, vi, sizeof(size_t));
        n = n->next;
      }
    }
  }

  free(vi);
  return scc;
}

bool graph_is_empty(graph g) {
  for (size_t i = 0; i < g->v; i++) {
    if (!g->removed[i])
      return false;
  }
  return true;
}

void graph_colorSCC(graph g) {
  size_t *colors = malloc(g->v * sizeof(size_t));
  while (!graph_is_empty(g)) {
    for (size_t v = 0; v < g->v; v++) {
      if (!g->removed[v]) {
        colors[v] = v;
      } else {
        colors[v] = g->scc_id[v];
      }
    }

    bool color_changed = true;
    size_t *old_colors = malloc(g->v * sizeof(size_t));
    while (color_changed) {
      color_changed = false;
      memcpy(old_colors, colors, g->v * sizeof(size_t));

      size_t w;
      cilk_for(size_t u = 0; u < g->v; u++) {
        if (!g->removed[u]) {
          for (node u_edges = g->edges[u]; u_edges != NULL;
               u_edges = u_edges->next) {
            w = node_peek_int(u_edges);
            if (colors[u] > colors[w]) {
              colors[w] = colors[u];
            }
          }
        }
      }

      if (memcmp(old_colors, colors, g->v * sizeof(size_t)) != 0) {
        color_changed = true;
      }
    }

    cilk_for(size_t i = 0; i < g->v; i++) {
      if (!g->removed[i] && colors[i] == i) {
        node scc = graph_bfs(g, colors[i], colors);
        size_t *vc = malloc(sizeof(size_t));
        while (scc != NULL) {
          node_pop(&scc, vc, sizeof(size_t));
          g->removed[*vc] = true;
          g->scc_id[*vc] = colors[i];
        }
      }
    }
  }
  free(colors);
  return;
}
