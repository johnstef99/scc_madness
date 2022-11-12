/*
 *
 * Created by:   github.com/johnstef99
 * Last updated: 2022-11-12
 *
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../libs/mmio.h"
#include "graph.h"
#include "node.h"
#include "utils.h"

void fill_graph_from_file(char *path, graph *g) {
  FILE *file = NULL;
  MM_typecode matcode;

  file = fopen(path, "r");
  if (file == NULL) {
    fprintf(stderr, "Couldn't open matrix file %s\n", path);
    exit(1);
  }

  if (mm_read_banner(file, &matcode) != 0) {
    fprintf(stderr, "Couldn't proccess matrix file\n");
    exit(1);
  }

  if (mm_is_valid(matcode) != 1) {
    fprintf(stderr, "Matrix not valid");
    exit(1);
  }

  int M, N, nz;
  int i, *I, *J;
  int *val;

  if ((mm_read_mtx_crd_size(file, &M, &N, &nz)) != 0)
    exit(1);

  *g = graph_new(M);

  I = (int *)malloc(nz * sizeof(int));
  J = (int *)malloc(nz * sizeof(int));
  val = (int *)malloc(nz * sizeof(int));

  for (i = 0; i < nz; i++) {
    fscanf(file, "%d %d %d\n", &I[i], &J[i], &val[i]);
    I[i]--; /* adjust from 1-based to 0-based */
    J[i]--;
  }

  if (file != stdin)
    fclose(file);

  for (i = 0; i < nz; i++)
    graph_add_edge(g, I[i], J[i]);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s [directory-of-matricies]\n", argv[0]);
    exit(1);
  }

  graph g = NULL;
  fill_graph_from_file(argv[1], &g);

  graph_free(&g);

  return 0;
}
