/*
 *
 * Created by:   github.com/johnstef99
 * Last updated: 2022-11-18
 *
 */

#ifndef NODE_H
#define NODE_H

#include <stdlib.h>
#include <stdint.h>

typedef int *point_to_i;

struct Node {
  void *data;
  struct Node *next;
};

typedef uint8_t *pByte;

typedef struct Node *node;

node node_new(void *data, size_t data_size);

uint8_t node_push(node *head, void *data, size_t data_size);

uint8_t node_pop(node *head, void *data, size_t data_size);

uint8_t node_peek(node head, void *data, size_t data_size);

size_t node_peek_int(node head);

uint8_t node_free(node *head);

#endif

