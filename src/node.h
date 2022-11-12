/*
 *
 * Created by:   github.com/johnstef99
 * Last updated: 2022-11-12
 *
 */

#ifndef NODE_H
#define NODE_H

struct Node {
  int data;
  struct Node *next;
};

typedef struct Node *node;

node node_new(int data);

int node_is_empty(node n);

void node_push(node *n, int data);

int node_pop(node *root);

int node_peak(node root);

void node_free(node *root);

#endif
