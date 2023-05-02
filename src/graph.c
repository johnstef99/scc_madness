/*
 *
 * Created by:   github.com/johnstef99
 * Last updated: 2022-11-22
 *
 */

#include "graph.h"

void size_t_zero(void *view) { *(size_t *)view = 0; }
void size_t_add(void *left, void *right) {
  *(size_t *)left += *(size_t *)right;
}
void size_t_destroy(void *view) { free(view); }

size_t cilk_reducer(size_t_zero, size_t_add) trimmed_red = 0;

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
  if (!g->removed) {
    fprintf(stderr, "Couldn't allocate removed array\n");
    exit(1);
  }

  g->n_trimmed = __builtin_addressof(trimmed_red);

  g->scc_id = malloc(g->v * sizeof(size_t));
  if (!g->scc_id) {
    fprintf(stderr, "Couldn't allocate scc_id array\n");
    exit(1);
  }

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

void graph_trim(graph g) {
  bool *has_in = calloc(g->v, sizeof(bool));
  if (!has_in) {
    fprintf(stderr, "Couldn't allocate has_in array\n");
    exit(1);
  }

  bool *has_out = calloc(g->v, sizeof(bool));
  if (!has_out) {
    fprintf(stderr, "Couldn't allocate has_out array\n");
    exit(1);
  }

  cilk_for(size_t v = 0; v < g->v; v++) {
    for (size_t j = g->in->com[v]; j < g->in->com[v + 1]; j++) {
      if (!g->removed[v] && g->in->unc[j] != v) {
        has_in[v] = true;
        has_out[g->in->unc[j]] = true;
      }
    }
  }

  cilk_for(size_t v = 0; v < g->v; v++) {
    if (!has_in[v] || !has_out[v]) {
      g->removed[v] = true;
      *(g->n_trimmed) += 1;
    }
  }
  free(has_in);
  free(has_out);
}

void graph_bfs(graph g, size_t entry, size_t *colors) {
  g->removed[entry] = true;

  size_t *fifo = malloc(g->e * sizeof(size_t));
  if(!fifo){
    fprintf(stderr, "Couldn't allocate fifo array\n");
    exit(1);
  }

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

void bool_false(void *view) { *(bool *)view = false; }
void bool_or(void *left, void *right) { *(bool *)left |= *(bool *)right; }
void bool_destroy(void *view) { free(view); }

void graph_colorSCC(graph g) {
  size_t *colors = malloc(g->v * sizeof(size_t));
  if(!colors){
    fprintf(stderr, "Couldn't allocate colors array\n");
    exit(1);
  }

  while (!graph_is_empty(g)) {
    cilk_for (size_t v = 0; v < g->v; v++) {
      colors[v] = g->scc_id[v];
    }

    bool cilk_reducer(bool_false, bool_or) color_changed = true;
    size_t w;
    while (color_changed) {
      color_changed = false;
      cilk_for(size_t u = 0; u < g->v; u++) {
        if (!g->removed[u]) {
          for (size_t j = g->in->com[u]; j < g->in->com[u + 1]; j++) {
            w = g->in->unc[j];
            if (g->removed[w])
              continue;
            if (colors[w] < colors[u]) {
              colors[u] = colors[w];
              color_changed |= true;
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
