/*
 *
 * Created by:   github.com/johnstef99
 * Last updated: 2022-11-25
 *
 */

#define NTHREADS 8

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
  if (!g->removed) {
    fprintf(stderr, "Couldn't allocate removed array\n");
    exit(1);
  }

  g->n_trimmed = 0;

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

typedef struct trim_params {
  graph g;
  size_t from, to, trimmed;
  bool *has_in;
  bool *has_out;
} trim_params;

void *_graph_trim(void *arg) {
  trim_params *params = (trim_params *)arg;
  graph g = params->g;

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

  for (size_t v = params->from; v < params->to; v++) {
    for (size_t j = g->in->com[v]; j < g->in->com[v + 1]; j++) {
      if (!g->removed[v] && g->in->unc[j] != v) {
        has_in[v] = true;
        has_out[g->in->unc[j]] = true;
      }
    }
  }
  return NULL;
}

void *_check_and_remove(void *args) {
  trim_params *params = (trim_params *)args;
  graph g = params->g;
  params->trimmed = 0;
  bool *has_in = params->has_in;
  bool *has_out = params->has_out;

  for (size_t v = params->from; v < params->to; v++) {
    if (!has_in[v] || !has_out[v]) {
      g->removed[v] = true;
      params->trimmed++;
    }
  }
  return NULL;
}

void graph_trim(graph g, int repeat) {
  pthread_t thread_id[NTHREADS];
  trim_params *thread_param[NTHREADS];
  size_t vert_per_thread = g->v / NTHREADS;
  int i;

  size_t trimmed_by_repeat = 0;
  for (int r = 0; r < repeat; r++) {
    bool *has_in = calloc(g->v, sizeof(bool));
    bool *has_out = calloc(g->v, sizeof(bool));
    for (i = 0; i < NTHREADS; i++) {
      thread_param[i] = malloc(sizeof(thread_param));
      if (!thread_param[i]) {
        fprintf(stderr, "Couldn't allocate thread_params for thread %d\n", i);
        exit(1);
      }

      thread_param[i]->g = g;
      thread_param[i]->has_in = has_in;
      thread_param[i]->has_out = has_out;
      thread_param[i]->from = i * vert_per_thread;
      thread_param[i]->to = thread_param[i]->from + vert_per_thread;
      if (i == NTHREADS - 1)
        thread_param[i]->to += g->v % NTHREADS;
      pthread_create(&thread_id[i], NULL, _graph_trim, (void *)thread_param[i]);
    }

    for (i = 0; i < NTHREADS; i++) {
      pthread_join(thread_id[i], NULL);
    }

    for (i = 0; i < NTHREADS; i++) {
      pthread_create(&thread_id[i], NULL, _check_and_remove,
                     (void *)thread_param[i]);
    }

    for (i = 0; i < NTHREADS; i++) {
      pthread_join(thread_id[i], NULL);
      trimmed_by_repeat += thread_param[i]->trimmed;
      free(thread_param[i]);
    }
    g->n_trimmed = trimmed_by_repeat;
    free(has_in);
    free(has_out);
    if (trimmed_by_repeat == 0)
      break;
  }
}

void graph_bfs(graph g, size_t entry, size_t *colors) {
  g->removed[entry] = true;

  size_t *fifo = malloc(g->e * sizeof(size_t));
  if (!fifo) {
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

typedef struct change_color_args {
  graph g;
  size_t from, to;
  size_t *colors;
  bool *color_changed;
} change_color_args;

void *_change_color(void *args) {
  change_color_args *ccags = (change_color_args *)args;
  graph g = ccags->g;
  size_t *colors = ccags->colors;
  size_t w;
  for (size_t u = ccags->from; u < ccags->to; u++) {
    if (!g->removed[u]) {
      for (size_t j = g->in->com[u]; j < g->in->com[u + 1]; j++) {
        w = g->in->unc[j];
        if (g->removed[w])
          continue;
        if (colors[w] < colors[u]) {
          *ccags->color_changed = true;
          colors[u] = colors[w];
        }
      }
    }
  }
  return NULL;
}

typedef struct bfs_args {
  graph g;
  size_t from, to;
  size_t *colors;
} bfs_args;

void *_graph_bfs(void *args) {
  bfs_args *bfsa = (bfs_args *)args;
  graph g = bfsa->g;
  size_t *colors = bfsa->colors;
  for (size_t i = bfsa->from; i < bfsa->to; i++) {
    if (!g->removed[i] && colors[i] == i) {
      graph_bfs(g, colors[i], colors);
    }
  }
  return NULL;
}

void graph_colorSCC(graph g) {
  size_t *colors = malloc(g->v * sizeof(size_t));
  if (!colors) {
    fprintf(stderr, "Couldn't allocate colors array\n");
    exit(1);
  }

  while (!graph_is_empty(g)) {
    for (size_t v = 0; v < g->v; v++) {
      if (!g->removed[v]) {
        colors[v] = v;
      } else {
        colors[v] = g->scc_id[v];
      }
    }

    pthread_t thread_id[NTHREADS];
    size_t vert_per_thread = g->v / NTHREADS;

    change_color_args *ccargs[NTHREADS];
    bool color_changed = true;
    while (color_changed) {
      color_changed = false;
      for (int i = 0; i < NTHREADS; i++) {
        ccargs[i] = malloc(sizeof(change_color_args));
        ccargs[i]->g = g;
        ccargs[i]->colors = colors;
        ccargs[i]->color_changed = &color_changed;
        ccargs[i]->from = i * vert_per_thread;
        ccargs[i]->to = ccargs[i]->from + vert_per_thread;
        if (i == NTHREADS - 1)
          ccargs[i]->to += g->v % NTHREADS;
        pthread_create(&thread_id[i], NULL, _change_color, ccargs[i]);
      }

      for (int i = 0; i < NTHREADS; i++) {
        pthread_join(thread_id[i], NULL);
        free(ccargs[i]);
      }
    }

    bfs_args *bfsa[NTHREADS];
    for (int i = 0; i < NTHREADS; i++) {
      bfsa[i] = malloc(sizeof(bfs_args));
      bfsa[i]->g = g;
      bfsa[i]->colors = colors;
      bfsa[i]->from = i * vert_per_thread;
      bfsa[i]->to = bfsa[i]->from + vert_per_thread;
      if (i == NTHREADS - 1)
        bfsa[i]->to += g->v % NTHREADS;
      pthread_create(&thread_id[i], NULL, _graph_bfs, bfsa[i]);
    }

    for (int i = 0; i < NTHREADS; i++) {
      pthread_join(thread_id[i], NULL);
      free(bfsa[i]);
    }
  }

  free(colors);
  return;
}
