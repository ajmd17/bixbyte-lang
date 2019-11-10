#pragma once

typedef struct {
  void *ptr;
  // @TODO hashmap of fields?
} object_t;

object_t *object_create();
void object_destroy(object_t *obj);
