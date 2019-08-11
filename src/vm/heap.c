#include <vm/heap.h>

heap_node_t *heap_node_create() {
  heap_node_t *node = (heap_node_t*)malloc(sizeof(heap_node_t));
  node->hv.ptr = NULL;
  node->hv.flags = 0;
  node->prev = NULL;
  node->next = NULL;
  return node;
}

void heap_node_destroy(heap_node_t *node) {
  // @TODO how to free internal obj? dtor pointer?
  free(node);
}

heap_t *heap_create() {
  heap_t *heap = (heap_t*)malloc(sizeof(heap_t));
  heap->head = NULL;
  heap->size = 0;
  return heap;
}

void heap_destroy(heap_t *heap) {
  while (heap->head) {
    heap_node_t *tmp = heap->head;
    heap->head = tmp->prev;

    heap_node_destroy(tmp);

    --heap->size;
  }
}

heap_value_t *heap_alloc(heap_t *heap) {
  pthread_mutex_lock(&heapMutex);

  heap_node_t *node = heap_node_create();

  if (heap->head != NULL) {
    heap->head->next = node;
  }

  // swap
  node->prev = heap->head;
  heap->head = node;

  ++heap->size;

  pthread_mutex_unlock(&heapMutex);

  return &heap->head->hv;
}

void heap_sweep(heap_t *heap) {
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

    heap_node_destroy(last);
    last = prev;

    --heap->size;
  }
}
