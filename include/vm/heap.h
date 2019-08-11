#pragma once

#include <stdint.h>

typedef struct {
  void *ptr;
  uint8_t flags;
} heap_value_t;

struct heap_node;
typedef struct heap_node heap_node_t;

struct heap_node {
  heap_value_t hv;
  heap_node_t *prev;
  heap_node_t *next;
};

heap_node_t *heap_node_create();
void heap_node_destroy(heap_node_t *node);

typedef struct {
  heap_node_t *head;
  size_t size;
} heap_t;

heap_t *heap_create();
void heap_destroy(heap_t *heap);

heap_value_t *heap_alloc(heap_t *heap);
void heap_sweep(heap_t *heap);
