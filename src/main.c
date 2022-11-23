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
#include "csx.h"
#include "fifo.h"
#include "graph.h"
#include "node.h"

int main(int argc, char *argv[]) {

  if (argc < 2) {
    fprintf(stderr, "Usage: %s [martix-market-filename]\n", argv[0]);
    exit(1);
  }

  csx csc = csc_from_file(argv[1]);
  graph g = graph_new_from_csc(csc);
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
