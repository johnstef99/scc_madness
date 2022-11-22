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
  size_t i, *I, *J;

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

  I = (size_t *)malloc(nz * sizeof(size_t));
  J = (size_t *)malloc(nz * sizeof(size_t));

  for (i = 0; i < nz; i++) {
    fscanf(f, "%zu %zu\n", &I[i], &J[i]);
  }

  if (f != stdin)
    fclose(f);

  mm_write_banner(stdout, matcode);
  mm_write_mtx_crd_size(stdout, M, N, nz);

  graph g = graph_new(M);
  for (i = 0; i < nz; i++) {
    graph_add_edge(g, I[i] - 1, J[i] - 1);
  }
  return g;
}

int main(int argc, char *argv[]) {

  if (argc < 2) {
    fprintf(stderr, "Usage: %s [martix-market-filename]\n", argv[0]);
    exit(1);
  }

  graph g = fill_graph_from_file(argv[1]);
  graph_trim(g);
  printf("Trimmed: %zu vertices\n", g->n_trimmed);

  time_t start = time(NULL);
  graph_colorSCC(g);
  time_t end = time(NULL);

  printf("colorSCC time: %fs\n", (double)(end - start));

  int num_of_scc = 0;
  for (int i = 0; i < g->v; i++) {
    if (g->scc_id[i] == i) {
      num_of_scc++;
    }
  }

  printf("Num of scc: %d\n", num_of_scc);

  return 0;
}
