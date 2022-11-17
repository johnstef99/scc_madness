/*
 *
 * Created by:   github.com/johnstef99
 * Last updated: 2022-11-18
 *
 */

#include "fifo.h"

fifo fifo_new() {
  fifo q = malloc(sizeof(struct FIFO));
  q->head = NULL;
  q->tail = NULL;
  return q;
}

uint8_t fifo_enqueue(fifo q, void *data, size_t data_size) {
  node n = NULL;
  uint8_t e = node_push(&n, data, data_size);
  if (q->head == NULL) {
    q->head = q->tail = n;
  } else {
    q->tail->next = n;
    q->tail = n;
  }
  return e;
}

uint8_t fifo_dequeue(fifo q, void *data, size_t data_size) {
  return node_pop(&(q->head), data, data_size);
};

uint8_t fifo_is_empty(fifo q) { return q->head == NULL; }
