#include <vm/runtime.h>

#include <assert.h>

runtime_t *runtime_create() {
  runtime_t *r = (runtime_t*)malloc(sizeof(runtime_t));

  r->heap = heap_create();
  r->dt = datatable_create();
  r->rc = rcmap_create();

  return r;
}

void runtime_destroy(runtime_t *r) {
  rcmap_destroy(r->rc);
  datatable_destroy(r, r->dt);
  heap_destroy(r->heap);
  free(r);
}

void runtime_gc(runtime_t *r) {
  heap_lock();

  datatable_mark(r->dt);
  heap_sweep(r->heap);

  heap_unlock();
}

void runtime_throwException(runtime_t *r, exception_t *e) {
  // @TODO internal VM handling.

}

refcounted_t runtime_claim(runtime_t *rt, rcmap_key_t key) {
  size_t *cnt = NULL;

  int result = rcmap_getPtr(rt->rc, key, &cnt);

  if (result == RC_MAP_OK) {
    ++(*cnt);
    return (refcounted_t)key;
  } else if (result == RC_MAP_MISSING) {
    result = rcmap_put(rt->rc, key, 1);
  }

  if (result == RC_MAP_OK) {
    return (refcounted_t)key;
  }

  // uh oh. throw exception?
  return NULL;
}

void runtime_release(runtime_t *rt, refcounted_t rc) {
  size_t *cnt = NULL;

  int result = rcmap_getPtr(rt->rc, rc, &cnt);

  if (result != RC_MAP_OK) {
    // throw exception?
    return;
  }

  assert(*cnt > 0);

  if (--(*cnt) == 0) {
    free(rc);

    result = rcmap_remove(rt->rc, rc);
  }

  if (result != RC_MAP_OK) {
    // throw
    return;
  }
}
