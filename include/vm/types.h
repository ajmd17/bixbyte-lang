#pragma once

typedef struct runtime runtime_t;
typedef struct value value_t;
typedef struct heap_value heap_value_t;
typedef struct heap heap_t;

typedef struct storage storage_t;

typedef struct args {
  storage_t *_stack;
  void *_rawData;
} args_t;

typedef uint32_t metadata_t;
typedef value_t (*native_function_t)(runtime_t*, args_t*);
