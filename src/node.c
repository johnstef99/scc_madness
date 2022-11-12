/*
 *
 * Created by:   github.com/johnstef99
 * Last updated: 2022-11-12
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "node.h"

node node_new(int data) {
  node n = (node)malloc(sizeof(node));
  n->data = data;
  n->next = NULL;
  return n;
}

int node_is_empty(node n) { return !n; }

void node_push(node *root, int data) {
  node n = node_new(data);
  n->next = *root;
  *root = n;
}

int node_pop(node *root) {
  if (node_is_empty(*root))
    return INT32_MIN;
  node temp = *root;
  *root = (*root)->next;
  int popped = temp->data;
  free(temp);
  return popped;
}

int node_peak(node root) {
  if (node_is_empty(root))
    return INT32_MIN;
  return root->data;
}

void node_free(node *root) {
  while (!node_is_empty(*root)) {
    node_pop(root);
  }
}
