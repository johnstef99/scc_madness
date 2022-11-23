/*
 *
 * Created by:   github.com/johnstef99
 * Last updated: 2022-11-23
 *
 */

#include "csx.h"

csx csx_new(size_t vertices, size_t edges) {
  csx csc = malloc(sizeof(struct CSX));
  csc->v = vertices;
  csc->e = edges;
  csc->com = calloc((vertices + 1), sizeof(size_t));
  csc->unc = calloc(edges, sizeof(size_t));

  if (!csc || !csc->com || !csc->unc) {
    fprintf(stderr, "Counldn't create CSC for %lu vertices and %lu edges\n",
            vertices, edges);
    exit(1);
  }
  return csc;
}

csx csx_transpose(csx original) {
  csx tran = csx_new(original->v, original->e);

  // count how many vertices each vertice points to or is pointed from
  size_t *counter = calloc(original->v, sizeof(size_t));
  for (size_t i = 0; i < original->e; i++) {
    counter[original->unc[i]]++;
  }

  // make the compressed row/col
  for (size_t i = 1; i < original->v + 1; i++) {
    tran->com[i] = tran->com[i - 1] + counter[i - 1];
  }

  // make the non-compressed col/row
  for (size_t i = 0; i < original->v; i++) {
    for (size_t j = original->com[i]; j < original->com[i + 1]; j++) {
      size_t points_to = original->unc[j];
      size_t dest = tran->com[points_to];

      tran->unc[dest] = i;
      tran->com[points_to]++;
    }
  }

  // correct the modified compressed row/col
  for (size_t col = 0, last = 0; col <= tran->v; col++) {
    size_t temp = tran->com[col];
    tran->com[col] = last;
    last = temp;
  }

  free(counter);
  return tran;
}

void csx_print(csx matrix) {
  for (size_t i = 0; i < matrix->e; i++) {
    printf("%3lu ", i);
  }
  puts("");
  for (size_t i = 0; i < matrix->e; i++) {
    printf("----");
  }
  puts("");
  for (size_t i = 0; i < matrix->v + 1; i++) {
    printf("%3lu ", matrix->com[i]);
  }
  puts("");
  for (size_t i = 0; i < matrix->e; i++) {
    printf("%3lu ", matrix->unc[i]);
  }
  puts("");
}

/*
 * Function:  csc_from_file
 * ------------------------
 * Reads a .mtx file and stores it in CSC format
 *
 * path: the path of the file to read the matrix from
 *
 * returns: The matrix in a CSC format
 */
csx csc_from_file(char *path) {
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

  size_t a, b;
  size_t ci = 0;
  csx csc = csx_new(M, nz);
  for (size_t i = 0; i < nz; i++) {
    fscanf(f, "%zu %zu\n", &a, &b);
    if (ci != b - 1) {
      csc->com[++ci] = i;
    }
    csc->unc[i] = a - 1;
  }
  csc->com[++ci] = nz;

  if (f != stdin)
    fclose(f);

  mm_write_banner(stdout, matcode);
  mm_write_mtx_crd_size(stdout, M, N, nz);
  puts("--------------");

  return csc;
}
