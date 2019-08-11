#include <vm/datatable.h>


datatable_t *datatable_create() {
  datatable_t *dt = (datatable_t*)malloc(sizeof(datatable_t));

  dt->storage[0].data = NULL;
  dt->storage[0].len = 0;

  dt->storage[AT_DATA].data = (value_t*)malloc(STATIC_DATA_SIZE_BYTES);
  memset(dt->storage[AT_DATA].data, 0, STATIC_DATA_SIZE_BYTES);
  dt->storage[AT_DATA].len = 0;

  dt->storage[AT_LOCAL].data = (value_t*)malloc(STACK_SIZE_BYTES);
  memset(dt->storage[AT_LOCAL].data, 0, STACK_SIZE_BYTES);
  dt->storage[AT_LOCAL].len = 0;

  dt->storage[AT_REG].data = (value_t*)malloc(sizeof(value_t) * NUM_REGISTERS);
  memset(dt->storage[AT_REG].data, 0, sizeof(value_t) * NUM_REGISTERS);
  dt->storage[AT_REG].len = 0;

  return dt;
}

void datatable_destroy(runtime_t *rt, datatable_t *dt) {
  int i;

  for (i = 0; i < 4; i++) {
    while (dt->storage[i].len) {
      value_destroy(rt, &dt->storage[i].data[--dt->storage[i].len]);
    }

    free(dt->storage[i].data);
  }

  free(dt);
}

void datatable_markTable(storage_t *s) {
  size_t len = s->len;

  while (len) {
    value_t *v = &s->data[len - 1];

    // @TODO partitioning to prevent branch prediction misses?
    if (v->metadata & (TYPE_POINTER | (FLAG_OBJECT << 8))) {
      v->data.hv->flags |= FLAG_MARKED;
    }

    --len;
  }
}

void datatable_mark(datatable_t *dt) {
  datatable_markTable(&dt->storage[AT_DATA]);
  datatable_markTable(&dt->storage[AT_LOCAL]);
}

value_t *datatable_getValue(datatable_t *dt, loc_28_t loc, archtype_t at) {
  storage_t *s = &dt->storage[at & 0x3];

  int abs = ((at & 0xC) >> 2) - 2;
  size_t idx = (-abs & loc) | ((abs - 1) & (s->len - loc));

  return &s->data[idx];
}
