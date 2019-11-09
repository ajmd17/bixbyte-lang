#include <vm/object.h>

#include <stdlib.h>

object_t *object_create() {
  object_t *obj = (object_t*)malloc(sizeof(object_t));
  return obj;
}

void object_destroy(object_t *obj) {
  // @TODO delete fields, call destructor ?

  free(obj);
}
