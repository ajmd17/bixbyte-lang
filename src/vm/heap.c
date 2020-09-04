#include <vm/heap.h>
#include <vm/value.h>
#include <vm/runtime.h>

#include <stdlib.h>

#include <pthread.h>
pthread_mutex_t heapMutex = PTHREAD_MUTEX_INITIALIZER;

heap_node_t *heap_node_create() {
  heap_node_t *node = (heap_node_t*)malloc(sizeof(heap_node_t));
  node->hv.ptr = NULL;
  node->hv.flags = 0;
  node->hv.dtor_ptr = NULL;
  node->prev = NULL;
  node->next = NULL;
  return node;
}

void heap_node_destroy(runtime_t *rt, heap_node_t *node) {
  if (node->hv.dtor_ptr != NULL) {
    args_t args;
    args._stack = &rt->dt->storage[AT_LOCAL];
    args._rawData = node->hv.ptr; // 'this' object

    node->hv.dtor_ptr(rt, &args);
  }

  free(node);
}

heap_t *heap_create() {
  heap_t *heap = (heap_t*)malloc(sizeof(heap_t));
  heap->head = NULL;
  heap->size = 0;
  return heap;
}

void heap_destroy(runtime_t *rt, heap_t *heap) {
  while (heap->head) {
    heap_node_t *tmp = heap->head;
    heap->head = tmp->prev;

    heap_node_destroy(rt, tmp);

    --heap->size;
  }
}

heap_value_t *heap_alloc(runtime_t *rt, heap_t *heap) {
  heap_lock();

  heap_node_t *node = heap_node_create();

  if (heap->head != NULL) {
    heap->head->next = node;
  }

  // swap
  node->prev = heap->head;
  heap->head = node;

  ++heap->size;

  heap_unlock();

  return &heap->head->hv;
}

void heap_sweep(runtime_t *rt, heap_t *heap) {
  heap_node_t *last = heap->head;

  while (last) {
    if (last->hv.flags & FLAG_MARKED) {
      // unmark
      last->hv.flags &= ~FLAG_MARKED;
      last = last->prev;
      continue;
    }

    // not marked; delete
    heap_node_t *prev = last->prev;
    heap_node_t *next = last->next;

    if (prev) {
      prev->next = next;
    }

    if (next) {
      // removing an item from the middle, so
      // make the nodes to the other sides now
      // point to each other
      next->prev = prev;
    } else {
      // since there are no nodes after this,
      // set the head to be this node here
      heap->head = prev;
    }

    heap_node_destroy(rt, last);
    last = prev;

    --heap->size;
  }
}

// TODO: make heap lock()/unlock() per heap so we can have multiple

void heap_lock() {
  pthread_mutex_lock(&heapMutex);
}

void heap_unlock() {
  pthread_mutex_unlock(&heapMutex);
}
