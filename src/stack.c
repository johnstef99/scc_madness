/*
 *
 * Created by:   github.com/johnstef99
 * Last updated: 2022-11-10
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "stack.h"

node s_new_node(int data) {
  node n = (node)malloc(sizeof(node));
  n->data = data;
  n->next = NULL;
  return n;
}

int s_is_empty(node n) { return !n; }

void s_push(node *root, int data) {
  node n = s_new_node(data);
  n->next = *root;
  *root = n;
}

void s_push_back(node *root, int data) {
  node n = s_new_node(data);

  if (s_is_empty(*root)) {
    s_push(root, data);
    return;
  }

  node *last = &((*root)->next);
  while (!s_is_empty(*last)) {
    last = &((*last)->next);
  }
  *last = n;
}

int s_pop(node *root) {
  if (s_is_empty(*root))
    return INT32_MIN;
  node temp = *root;
  *root = (*root)->next;
  int popped = temp->data;
  free(temp);
  return popped;
}

int s_peek(node *root) {
  if (s_is_empty(*root))
    return INT32_MIN;
  return (*root)->data;
}
