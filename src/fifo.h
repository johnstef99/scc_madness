/*
 *
 * Created by:   github.com/johnstef99
 * Last updated: 2022-11-18
 *
 */

#ifndef FIFO_H
#define FIFO_H

#include <stdint.h>

#include "node.h"

struct FIFO {
  node head;
  node tail;
};

typedef struct FIFO *fifo;

fifo fifo_new();

uint8_t fifo_enqueue(fifo q, void *data, size_t data_size);

uint8_t fifo_dequeue(fifo q, void *data, size_t data_size);

uint8_t fifo_is_empty(fifo q);

#endif
