/*
 *
 * Created by:   github.com/johnstef99
 * Last updated: 2022-11-10
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

void handle_truncated(char *src, char *dst) {
  fprintf(stderr, "%s + %s gets truncated\n", src, dst);
  exit(1);
}
