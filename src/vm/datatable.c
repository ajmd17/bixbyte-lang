#include <vm/datatable.h>
#include <vm/value.h>
#include <vm/heap.h>

#include <string.h>

datatable_t *datatable_create() {
  datatable_t *dt = (datatable_t*)malloc(sizeof(datatable_t));

  dt->storage[0].data = (value_t*)malloc(sizeof(value_t) * 32);
  for (int i = 0; i < 32; i++) {
    dt->storage[0].data[i].data.u64 = 0;
    dt->storage[0].data[i].metadata = TYPE_UINT;
  }
  dt->storage[AT_VM].lenVal = &VM_DATA_POINTER(dt);

  dt->storage[AT_DATA].data = (value_t*)malloc(STATIC_DATA_SIZE_BYTES);
  memset(dt->storage[AT_DATA].data, 0, STATIC_DATA_SIZE_BYTES);
  // dt->storage[AT_DATA].len = 0;
  dt->storage[AT_DATA].lenVal = &VM_STATIC_DATA_POINTER(dt);

  dt->storage[AT_LOCAL].data = (value_t*)malloc(STACK_SIZE_BYTES);
  memset(dt->storage[AT_LOCAL].data, 0, STACK_SIZE_BYTES);
  // dt->storage[AT_LOCAL].len = 0;
  dt->storage[AT_LOCAL].lenVal = &VM_STACK_POINTER(dt);

  dt->storage[AT_REG].data = (value_t*)malloc(sizeof(value_t) * NUM_REGISTERS);
  memset(dt->storage[AT_REG].data, 0, sizeof(value_t) * NUM_REGISTERS);
  // dt->storage[AT_REG].len = 0;
  dt->storage[AT_REG].lenVal = &VM_REG_POINTER(dt);

  return dt;
}

void datatable_destroy(runtime_t *rt, datatable_t *dt) {
  int i;

  for (i = 0; i < 4; i++) {
    while (*dt->storage[i].lenVal) {
      value_destroy(rt, &dt->storage[i].data[*dt->storage[i].lenVal - 1]);
      --*dt->storage[i].lenVal;
    }

    if (dt->storage[i].data != NULL) {
      free(dt->storage[i].data);
      dt->storage[i].data = NULL;
    }
  }

  free(dt);
}

void datatable_markTable(storage_t *s) {
  while (*s->lenVal) {
    value_t *v = &s->data[*s->lenVal - 1];

    // @TODO partitioning to prevent branch prediction misses?
    if (v->metadata & (TYPE_POINTER | (FLAG_OBJECT << 8))) {
      v->data.hv->flags |= FLAG_MARKED;
    }

    --*s->lenVal;
  }
}

void datatable_mark(datatable_t *dt) {
  datatable_markTable(&dt->storage[AT_DATA]);
  datatable_markTable(&dt->storage[AT_LOCAL]);
}

/* address [00000000 00000000 00000000 0000] abs/rel [00] storage [00] */
value_t *datatable_getValue(datatable_t *dt, loc_28_t loc, archtype_t at) {
  storage_t *s = &dt->storage[at & 0x3];

  int isabs = (at & AT_ABS) == AT_ABS;
  size_t idx = (-isabs & loc) | ((isabs - 1) & (*s->lenVal - loc));

  return &s->data[idx];
}
