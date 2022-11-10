/*
 *
 * Created by:   github.com/johnstef99
 * Last updated: 2022-11-10
 *
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../libs/mmio.h"
#include "stack.h"
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

  if (mm_read_banner(file, matcode) != 0) {
    fprintf(stderr, "Couldn't proccess matrix file\n");
    exit(1);
  }

  if (mm_is_valid(*matcode) != 1) {
    fprintf(stderr, "Matrix not valid");
    exit(1);
  }

  int M, N, nz;
  int i, *I, *J;
  int *val;

  if ((mm_read_mtx_crd_size(file, &M, &N, &nz)) != 0)
    exit(1);

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

  mm_write_banner(stdout, *matcode);
  mm_write_mtx_crd_size(stdout, M, N, nz);
  for (i = 0; i < nz; i++)
    fprintf(stdout, "%3d %3d %5d\n", I[i] + 1, J[i] + 1, val[i]);
}

struct Graph {
  int v;
  node *nodes;
};

typedef struct Graph *graph;

graph make_graph(int v) {
  graph g = (graph)malloc(sizeof(graph));
  g->v = v;
  g->nodes = malloc(v * sizeof(node));
  return g;
}

void addEdge(graph *g, int s, int d) { s_push(&((*g)->nodes[s]), d); }

void printAllVerticies(graph g) {
  for (int i = 0; i != g->v; i++) {
    printf("V%2d: ", i);
    node n = g->nodes[i];
    while (!s_is_empty(n)) {
      printf("%2d", n->data);
      n = n->next;
    }
    puts("");
  }
}

graph transpose(graph g) {
  graph t = make_graph(g->v);
  for (int i = 0; i != g->v; i++) {
    while (!s_is_empty(g->nodes[i])) {
      addEdge(&t, s_pop(&g->nodes[i]), i);
    }
  }
  return t;
}

void DFS(graph g, int s, bool visitedV[]) {
  visitedV[s] = true;
  printf("%d ", s);

  node n = g->nodes[s];
  while (!s_is_empty(n)) {
    if (!visitedV[n->data])
      DFS(g, n->data, visitedV);
    n = n->next;
  }
}

void fillOrder(graph g, int s, bool visitedV[], node *Stack) {
  visitedV[s] = true;

  node n = g->nodes[s];
  while (!s_is_empty(n)) {
    if (!visitedV[n->data])
      fillOrder(g, n->data, visitedV, Stack);
    n = n->next;
  }

  s_push(Stack, s);
}
void printSCC(graph g) {
  node Stack = malloc(sizeof(node));

  bool visitedV[g->v];
  for (int i = 0; i < g->v; i++)
    visitedV[i] = false;

  for (int i = 0; i < g->v; i++)
    if (visitedV[i] == false)
      fillOrder(g, i, visitedV, &Stack);

  graph gr = transpose(g);

  for (int i = 0; i < g->v; i++)
    visitedV[i] = false;

  while (!s_is_empty(Stack)) {
    int s = s_pop(&Stack);

    if (visitedV[s] == false) {
      DFS(gr, s, visitedV);
      puts("");
    }
  }
}

int main(int argc, char *argv[]) {
  graph g = make_graph(8);

  addEdge(&g, 0, 1);
  addEdge(&g, 1, 2);
  addEdge(&g, 2, 3);
  addEdge(&g, 2, 4);
  addEdge(&g, 3, 0);
  addEdge(&g, 4, 5);
  addEdge(&g, 5, 6);
  addEdge(&g, 6, 4);
  addEdge(&g, 6, 7);

  puts("\nPrint SCC");
  printSCC(g);

  /* if (argc < 2) { */
  /*   fprintf(stderr, "Usage: %s [directory-of-matricies]\n", argv[0]); */
  /*   exit(1); */
  /* } */

  /* MM_typecode matcode; */

  /* get_matrix(argv[1], &matcode); */

  return 0;
}
