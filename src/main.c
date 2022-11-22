/*
 *
 * Created by:   github.com/johnstef99
 * Last updated: 2022-11-18
 *
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../libs/mmio.h"
#include "fifo.h"
#include "graph.h"
#include "node.h"

graph fill_graph_from_file(char *path) {
  int ret_code;
  MM_typecode matcode;
  FILE *f;
  int M, N, nz;

  if ((f = fopen(path, "r")) == NULL) {
    fprintf(stderr, "Couldn't open file %s\n", path);
    exit(1);
  }

  if (mm_read_banner(f, &matcode) != 0) {
    printf("Could not process Matrix Market banner.\n");
    exit(1);
  }

  if ((ret_code = mm_read_mtx_crd_size(f, &M, &N, &nz)) != 0)
    exit(1);

  graph g = graph_new(M);
  size_t a, b;
  for (size_t i = 0; i < nz; i++) {
    fscanf(f, "%zu %zu\n", &a, &b);
    graph_add_edge(g, a - 1, b - 1);
  }

  if (f != stdin)
    fclose(f);

  mm_write_banner(stdout, matcode);
  mm_write_mtx_crd_size(stdout, M, N, nz);
  puts("--------");

  return g;
}

int main(int argc, char *argv[]) {

  if (argc < 2) {
    fprintf(stderr, "Usage: %s [martix-market-filename]\n", argv[0]);
    exit(1);
  }

  graph g = fill_graph_from_file(argv[1]);
  graph_trim(g);
  printf("Trimmed:\t %zu vertices\n", g->n_trimmed);

  time_t start = time(NULL);
  graph_colorSCC(g);
  time_t end = time(NULL);

  printf("colorSCC time:\t %fs\n", (double)(end - start));

  int num_of_scc = 0;
  for (int i = 0; i < g->v; i++) {
    if (g->scc_id[i] == i) {
      num_of_scc++;
    }
  }
  printf("Num of scc:\t %d\n", num_of_scc);

  return 0;
}
