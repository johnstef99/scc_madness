/*
 *
 * Created by:   github.com/johnstef99
 * Last updated: 2022-11-18
 *
 */

#include "node.h"

node node_new(void *data, size_t data_size) {
  node n = malloc(sizeof(struct Node));
  n->data = malloc(data_size);
  if (!n->data) {
    return NULL;
  }

  for (int i = 0; i < data_size; i++) {
    *(pByte)(n->data + i) = *(pByte)(data + i);
  }

  return n;
}

uint8_t node_push(node *head, void *data, size_t data_size) {
  node n = malloc(sizeof(struct Node));
  n->data = malloc(data_size);
  if (!n->data) {
    return 1;
  }

  n->next = *head;

  for (int i = 0; i < data_size; i++) {
    *(pByte)(n->data + i) = *(pByte)(data + i);
  }

  *head = n;

  return 0;
}

uint8_t node_pop(node *head, void *data, size_t data_size) {
  if (!*head)
    return 1;
  void *popped = (*head)->data;
  *head = (*head)->next;

  for (int i = 0; i < data_size; i++) {
    *(pByte)(data + i) = *(pByte)(popped + i);
  }

  free(popped);
  return 0;
}

uint8_t node_peek(node head, void *data, size_t data_size) {
  if (!head)
    return 1;
  void *peeked = head->data;

  for (int i = 0; i < data_size; i++) {
    *(pByte)(data + i) = *(pByte)(peeked + i);
  }

  return 0;
}

size_t node_peek_int(node head) {
  if (!head)
    return INT64_MIN;
  void *data = malloc(sizeof(size_t));

  for (size_t i = 0; i < sizeof(size_t); i++) {
    *(pByte)(data + i) = *(pByte)(head->data + i);
  }
  size_t out = *(size_t *)data;
  free(data);
  return out;
}

uint8_t node_free(node *head) {
  while (*head) {
    node_pop(head, NULL, 0);
  }
  return 0;
}

