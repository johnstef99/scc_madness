/*
 *
 * Created by:   github.com/johnstef99
 * Last updated: 2022-11-10
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../libs/mmio.h"
#include "utils.h"

#define MAXPATHLEN 256

void get_matrix(char *path, MM_typecode *matcode) {
  char *filename, pname[MAXPATHLEN];

  filename = "/celegansneural.mtx";

  if (strlcpy(pname, path, sizeof(pname)) >= sizeof(pname))
    handle_truncated(pname, path);
  if (strlcat(pname, filename, sizeof(pname)) >= sizeof(pname))
    handle_truncated(pname, filename);

  FILE *file = NULL;

  file = fopen(pname, "r");
  if (file == NULL) {
    fprintf(stderr, "Couldn't open matrix file %s\n", pname);
    exit(1);
  }

  int e = mm_read_banner(file, matcode);
  if (e != 0) {
    fprintf(stderr, "Couldn't proccess matrix file\n");
    exit(1);
  }

  puts(mm_is_valid(*matcode) ? "Matrix is valid" : "Matrix not valid");
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s [directory-of-matricies]\n", argv[0]);
    exit(1);
  }

  MM_typecode matcode;

  get_matrix(argv[1], &matcode);

  return 0;
}
