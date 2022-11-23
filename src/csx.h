/*
 *
 * Created by:   github.com/johnstef99
 * Last updated: 2022-11-23
 *
 */

#ifndef CSX_H
#define CSX_H

#include <stdio.h>
#include <stdlib.h>

#include "../libs/mmio.h"

/* Compressed Sparse Row/Column */
struct CSX {
  /* num of vertices */
  size_t v;
  /* num of edges */
  size_t e;
  /* compressed row/col */
  size_t *com;
  /* uncompressed col/row */
  size_t *unc;
};

typedef struct CSX *csx;

csx csx_new(size_t vertices, size_t edges);

csx csx_transpose(csx matrix);

void csx_print(csx matrix);

csx csc_from_file(char *path);

#endif
