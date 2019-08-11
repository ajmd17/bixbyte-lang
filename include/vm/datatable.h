#pragma once

#include <vm/value.h>

#include <stdint.h>

#define MB_TO_BYTES(mb) (mb*1000000)
#define DEFAULT_STATIC_DATA_SIZE_MB 5 // in MB
#define STATIC_DATA_SIZE_BYTES (MB_TO_BYTES(DEFAULT_STATIC_DATA_SIZE_MB) - (MB_TO_BYTES(DEFAULT_STATIC_DATA_SIZE_MB) % sizeof(value_t)))
#define DEFAULT_STACK_SIZE_MB 20 // in MB
#define STACK_SIZE_BYTES (MB_TO_BYTES(DEFAULT_STACK_SIZE_MB) - (MB_TO_BYTES(DEFAULT_STACK_SIZE_MB) % sizeof(value_t)))
#define NUM_REGISTERS 4

typedef struct {
  value_t *data;
  size_t len;
} storage_t;

typedef struct {
  storage_t storage[4];
} datatable_t;

datatable_t *datatable_create();
void datatable_destroy(runtime_t *rt, datatable_t *dt);
void datatable_markTable(storage_t *s);
void datatable_mark(datatable_t *dt);
value_t *datatable_getValue(datatable_t *dt, loc_28_t loc, archtype_t at);
