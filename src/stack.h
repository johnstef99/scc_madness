/*
 *
 * Created by:   github.com/johnstef99
 * Last updated: 2022-11-10
 *
 */

#ifndef STACK_H
#define STACK_H

struct Node {
  int data;
  struct Node *next;
};

typedef struct Node *node;

node s_new_node(int data);

int s_is_empty(node n);

void s_push(node *n, int data);

void s_push_back(node *n, int data);

int s_pop(node *root);

int s_peek(node *root);

#endif
