/*
 *
 * Created by:   github.com/johnstef99
 * Last updated: 2022-11-18
 *
 */

#include <mach/mach_time.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

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

  mach_timebase_info_data_t timeBase;
  mach_timebase_info(&timeBase);
  double timeConvert =
      (double)timeBase.numer / (double)timeBase.denom / 1000000;

  double start, elapsed;
  start = (double)mach_continuous_time() * timeConvert;
  elapsed = ((double)mach_continuous_time() * timeConvert) - start;

  start = (double)mach_continuous_time() * timeConvert;
  csx csc = csc_from_file(argv[1]);
  elapsed = (((double)mach_continuous_time() * timeConvert) - start);
  printf("MTX to CSC time: %f ms\n", (elapsed));

  graph g = graph_new_from_csc(csc);

  start = (double)mach_continuous_time() * timeConvert;
  graph_trim(g);
  elapsed = (((double)mach_continuous_time() * timeConvert) - start);

  printf("Trimmed:\t %zu vertices\n", g->n_trimmed);
  printf("Trimming time:\t %f ms\n", (elapsed));

  start = (double)mach_continuous_time() * timeConvert;
  graph_colorSCC(g);
  elapsed = (((double)mach_continuous_time() * timeConvert) - start);

  printf("colorSCC time:\t %f ms\n", elapsed);

  int num_of_scc = 0;
  for (int i = 0; i < g->v; i++) {
    if (g->scc_id[i] == i) {
      num_of_scc++;
    }
  }

  printf("Num of scc:\t %d\n", num_of_scc);

  return 0;
}
