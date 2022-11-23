/*
 *
 * Created by:   github.com/johnstef99
 * Last updated: 2022-11-22
 *
 */

#include "graph.h"
#include "fifo.h"
#include "node.h"

#include <cilk/cilk.h>

graph graph_new_from_csc(csx csc) {
  graph g = malloc(sizeof(struct Graph));
  if (!g) {
    fprintf(stderr, "Couldn't allocate graph with %lu vertices\n", csc->v);
    exit(1);
  }
  g->v = csc->v;
  g->e = csc->e;
  g->in = csc;
  g->out = csx_transpose(csc);
  g->removed = calloc(g->v, sizeof(bool));
  g->n_trimmed = 0;

  g->scc_id = malloc(g->v * sizeof(size_t));
  for (size_t i = 0; i < g->v; i++) {
    g->scc_id[i] = i;
  }

  return g;
}

/*
 * Returns the edge[j] of vertice i. This function doesn't check if the vertice
 * i has j amount of vertices, this check should be done before calling it.
 * For example:
 *
 * E(out_edges, 4, 0) -> will return the first vertice that vertice 4 points to.
 *
 * matrix: CSC/CSX matrix to get the data from
 * i: the vertice
 * j: the index of the edge
 *
 * returns: the vertice that points or is pointed from vertice i
 */
size_t E(csx edges, size_t i, size_t j) {
  return edges->unc[edges->com[i] + j];
}

size_t num_of_edges(csx edges, size_t v) {
  return edges->com[v + 1] - edges->com[v];
}

void graph_trim(graph g) {
  for (size_t v = 0; v < g->v; v++) {
    size_t in_degree = num_of_edges(g->in, v);
    bool zero_in = in_degree == 0 || (in_degree == 1 && E(g->in, v, 0) == v);

    size_t out_degree = g->out->com[v + 1] - g->out->com[v];
    bool zero_out =
        out_degree == 0 || (out_degree == 1 && E(g->out, v, 0) == v);

    if (zero_in || zero_out) {
      g->removed[v] = true;
      g->n_trimmed++;
    }
  }
}

void graph_bfs(graph g, size_t entry, size_t *colors) {
  g->removed[entry] = true;

  fifo q = fifo_new();
  for (size_t j = g->in->com[entry]; j < g->in->com[entry + 1]; j++) {
    fifo_enqueue(q, &g->in->unc[j], sizeof(size_t));
  }

  size_t *vi = malloc(sizeof(size_t));
  while (!fifo_is_empty(q)) {
    fifo_dequeue(q, vi, sizeof(size_t));

    if (!g->removed[*vi] && colors[*vi] == entry) {
      g->removed[*vi] = true;
      g->scc_id[*vi] = entry;

      for (size_t j = g->in->com[*vi]; j < g->in->com[*vi + 1]; j++) {
        fifo_enqueue(q, &g->in->unc[j], sizeof(size_t));
      }
    }
  }

  free(vi);
  return;
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

    _Atomic bool color_changed = true;
    size_t w;
    while (color_changed) {
      color_changed = false;
      cilk_for(size_t u = 0; u < g->v; u++) {
        if (!g->removed[u]) {
          for (size_t j = g->out->com[u]; j < g->out->com[u + 1]; j++) {
            w = g->out->unc[j];
            if (colors[u] > colors[w]) {
              colors[w] = colors[u];
              color_changed = true;
            }
          }
        }
      }
    }

    cilk_for(size_t i = 0; i < g->v; i++) {
      if (!g->removed[i] && colors[i] == i) {
        graph_bfs(g, colors[i], colors);
      }
    }
  }

  free(colors);
  return;
}
