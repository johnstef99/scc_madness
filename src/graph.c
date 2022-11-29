/*
 *
 * Created by:   github.com/johnstef99
 * Last updated: 2022-11-23
 *
 */

#include "graph.h"

graph graph_new_from_csc(csx csc) {
  graph g = malloc(sizeof(struct Graph));
  if (!g) {
    fprintf(stderr, "Couldn't allocate graph with %lu vertices\n", csc->v);
    exit(1);
  }
  g->v = csc->v;
  g->e = csc->e;
  g->in = csc;
  g->removed = calloc(g->v, sizeof(bool));
  g->n_trimmed = 0;

  g->scc_id = malloc(g->v * sizeof(size_t));
  for (size_t i = 0; i < g->v; i++) {
    g->scc_id[i] = i;
  }

  return g;
}

void graph_free(graph g) {
  free(g->in);
  free(g->removed);
  free(g->scc_id);
  free(g);
}

// clang-format off
void graph_trim(graph g) {
  size_t trimmed_per_thread;
  bool *has_in = calloc(g->v, sizeof(bool));
  bool *has_out = calloc(g->v, sizeof(bool));

  #pragma omp parallel private(trimmed_per_thread) shared(has_in, has_out)
  {
    trimmed_per_thread = 0;
    #pragma omp for
    {
      for (size_t v = 0; v < g->v; v++) {
        for (size_t j = g->in->com[v]; j < g->in->com[v+1]; j++) {
          if(!g->removed[v] && g->in->unc[j] != v){
            has_in[v] = true;
            has_out[g->in->unc[j]] = true;
          }
        }
      }
    }

    #pragma omp barrier

    #pragma omp for
    {
      for (size_t v = 0; v < g->v; v++) {
        if (!has_in[v] || !has_out[v]) {
          g->removed[v] = true;
          trimmed_per_thread++;
        }
      }
    }

    #pragma omp critical
    { g->n_trimmed += trimmed_per_thread; }
  }
  free(has_in);
  free(has_out);
} // clang-format on

void graph_bfs(graph g, size_t entry, size_t *colors) {
  g->removed[entry] = true;

  size_t *fifo = malloc(g->e * sizeof(size_t));
  size_t head = 0;
  size_t tail = 0;

  for (size_t j = g->in->com[entry]; j < g->in->com[entry + 1]; j++) {
    fifo[tail++] = g->in->unc[j];
  }

  size_t vi;
  while (head < tail) {
    vi = fifo[head++];

    if (!g->removed[vi] && colors[vi] == entry) {
      g->removed[vi] = true;
      g->scc_id[vi] = entry;

      for (size_t j = g->in->com[vi]; j < g->in->com[vi + 1]; j++) {
        fifo[tail++] = g->in->unc[j];
      }
    }
  }

  free(fifo);
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

    bool color_changed = true;
    size_t w;
    while (color_changed) {
      color_changed = false;

      // clang-format off
      #pragma omp parallel private(w) shared(color_changed)
      {
        #pragma omp for // clang-format on
        {
          for (size_t u = 0; u < g->v; u++) {
            if (!g->removed[u]) {
              for (size_t j = g->in->com[u]; j < g->in->com[u + 1]; j++) {
                w = g->in->unc[j];
                if (g->removed[w])
                  continue;
                if (colors[w] < colors[u]) {
                  color_changed = true;
                  colors[u] = colors[w];
                }
              }
            }
          }
        }
      }
    }

    // clang-format off
    #pragma omp parallel
    {
      #pragma omp for // clang-format on
      {
        for (size_t i = 0; i < g->v; i++) {
          if (!g->removed[i] && colors[i] == i) {
            graph_bfs(g, colors[i], colors);
          }
        }
      }
    }
  }

  free(colors);
  return;
}
