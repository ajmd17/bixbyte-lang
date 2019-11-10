#pragma once

#include <vm/datatable.h>
#include <vm/heap.h>
#include <vm/rc.h>
#include <vm/except.h>

typedef struct runtime runtime_t;

struct runtime {
  datatable_t *dt;
  heap_t *heap;
  rcmap_t *rc;
};

runtime_t *runtime_create();
void runtime_destroy(runtime_t *r);

void runtime_gc(runtime_t *r);

void runtime_throwException(runtime_t *r, exception_t *e);

refcounted_t runtime_claim(runtime_t *rt, rcmap_key_t key);
void runtime_release(runtime_t *rt, refcounted_t rc);
