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
#include <unistd.h>

#include "../libs/mmio.h"
#include "csx.h"
#include "fifo.h"
#include "graph.h"
#include "node.h"

double get_elapsed(struct timespec start, struct timespec end) {
  return (double)(end.tv_sec - start.tv_sec) * 1000 +
         (double)(end.tv_nsec - start.tv_nsec) / 1000000;
}

int main(int argc, char *argv[]) {


  if (argc < 2) {
    fprintf(stderr, "Usage: %s [martix-market-filename]\n", argv[0]);
    exit(1);
  }

  struct timespec start, end;
  double read_file, trimming, coloring;

  clock_gettime(CLOCK_MONOTONIC, &start);
  csx csc = csc_from_file(argv[1]);
  clock_gettime(CLOCK_MONOTONIC, &end);
  read_file = get_elapsed(start, end);
  printf("MTX to CSC time: %f ms\n", (read_file));

  graph g = graph_new_from_csc(csc);

  clock_gettime(CLOCK_MONOTONIC, &start);
  graph_trim(g);
  clock_gettime(CLOCK_MONOTONIC, &end);
  trimming = get_elapsed(start, end);

  printf("Trimmed:\t %zu vertices\n", g->n_trimmed);
  printf("Trimming time:\t %f ms\n", (trimming));

  clock_gettime(CLOCK_MONOTONIC, &start);
  graph_colorSCC(g);
  clock_gettime(CLOCK_MONOTONIC, &end);
  coloring = get_elapsed(start, end);

  printf("colorSCC time:\t %f ms\n", coloring);
  printf("Total time:\t %f ms\n", trimming + coloring);

  int num_of_scc = 0;
  for (int i = 0; i < g->v; i++) {
    if (g->scc_id[i] == i) {
      num_of_scc++;
    }
  }

  printf("Num of scc:\t %d\n", num_of_scc);

  printf("%f %zu %f %f %d\n", trimming, g->n_trimmed, coloring,
         trimming + coloring, num_of_scc);

  return 0;
}
