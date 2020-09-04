#pragma once

#include <stdint.h>
#include <stddef.h>

#include <vm/types.h>

typedef struct heap_value {
  void *ptr;
  native_function_t dtor_ptr;
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
void heap_node_destroy(runtime_t *rt, heap_node_t *node);

typedef struct heap {
  heap_node_t *head;
  size_t size;
} heap_t;

heap_t *heap_create();
void heap_destroy(runtime_t *rt, heap_t *heap);

heap_value_t *heap_alloc(runtime_t *rt, heap_t *heap);
void heap_sweep(runtime_t *rt, heap_t *heap);

void heap_lock();
void heap_unlock();
