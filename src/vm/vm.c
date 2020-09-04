#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <time.h>
#include <pthread.h>

// ===== Instructions =====

#include <vm/obj_loc.h>
#include <vm/rc.h>
#include <vm/value.h>
#include <vm/heap.h>
#include <vm/types.h>

#include <vm/jit.h>

#define MEASURE_EXECUTION_TIME_BEGIN clock_t begin = clock()
#define MEASURE_EXECUTION_TIME_END clock_t end = clock()
#define MEASURE_EXECUTION_TIME_RESULT (double)(end - begin) / CLOCKS_PER_SEC


// enum _VALUE_FLAGS;
// typedef enum _VALUE_FLAGS VALUE_FLAGS;
// value_t value_fromRawPointer(void*, VALUE_FLAGS);
// void value_destroy(runtime_t *rt, value_t *value);

// ===== heap memory =====





// ===== object =====


/*
typedef struct {
  char *key;
  bool used;
  value_t value; // payload
} objfield_t;

typedef struct {
  size_t tableSize;
  size_t size;
  objfield_t *data;
} objfieldmap_t;

objfieldmap_t *objfieldmap_create() {
  objfieldmap_t *map = (objfieldmap_t*)malloc(sizeof(objfieldmap_t));
  map->data = (objfield_t*)calloc(OBJFIELDMAP_INITIAL_SIZE, sizeof(objfield_t));
  map->tableSize = OBJFIELDMAP_INITIAL_SIZE;
  map->size = 0;
  return map;
}

void objfieldmap_destroy(objfieldmap_t *map) {
  free(map->data);
  free(map);
}

int objfieldmap_hash(objfieldmap_t *map, char *key) {
  int curr, i;

  if (map->size >= (map->tableSize / 2)) {
    return OBJFIELDMAP_FULL;
  }

  curr = objfieldmap_hashInt(map, key);

  for (i = 0; i < OBJFIELDMAP_CHAIN_LENGTH; i++) {
    if (!map->data[curr].used) {
      return curr;
    }

    if (map->data[curr].used && strcmp(map->data[curr].key, key) == 0) {
      return curr;
    }

    curr = (curr + 1) % map->tableSize;
  }

  return OBJFIELDMAP_FULL;
}

int objfieldmap_put(objfieldmap_t *map, char *key, value_t value);

int objfieldmap_rehash(objfieldmap_t *map) {
  int i, oldSize;
  objfield_t *curr, *tmp;

  tmp = (objfield_t*)calloc(2 * map->tableSize, sizeof(objfield_t));
  curr = map->data;
  map->data = tmp;

  oldSize = map->tableSize;
  map->tableSize *= 2;
  map->size = 0;

  for (i = 0; i < oldSize; i++) {
    int status;

    if (!curr[i].used) {
      continue;
    }

    status = objfieldmap_put(map, curr[i].key, curr[i].value);

    if (status != OBJFIELDMAP_OK) {
      return status;
    }
  }

  free(curr);

  return OBJFIELDMAP_OK;
}

int rcmap_put(rcmap_t *map, rcmap_key_t key, size_t count) {
  int index = rcmap_hash(map, key);

  while (index == RC_MAP_FULL) {
    if (rcmap_rehash(map) == RC_MAP_OMEM) {
      return RC_MAP_OMEM;
    }

    index = rcmap_hash(map, key);
  }

  map->data[index].key = key;
  map->data[index].count = count;
  map->data[index].used = true;
  map->size++;

  return RC_MAP_OK;
}

int rcmap_getPtr(rcmap_t *map, rcmap_key_t key, size_t **out) {
  int curr, i;

  curr = rcmap_hashInt(map, key);

  for (i = 0; i < RC_MAX_CHAIN_LENGTH; i++) {
    if (map->data[curr].used) {
      if (map->data[curr].key == key) {
        *out = &map->data[curr].count;

        return RC_MAP_OK;
      }
    }

    curr = (curr + 1) % map->tableSize;
  }

  *out = NULL;

  return RC_MAP_MISSING;
}

int rcmap_get(rcmap_t *map, rcmap_key_t key, size_t *out) {
  size_t *ptr = NULL;
  int result = rcmap_getPtr(map, key, &ptr);

  if (result == RC_MAP_OK) {
    *out = *ptr;
  } else {
    *out = 0;
  }

  return result;
}

int rcmap_remove(rcmap_t *map, rcmap_key_t key) {
  int curr, i;

  curr = rcmap_hashInt(map, key);

  for (i = 0; i < RC_MAX_CHAIN_LENGTH; i++) {
    if (map->data[curr].used) {
      if (map->data[curr].key == key) {
        memset(&map->data[curr], 0, sizeof(rcentry_t));
        --map->size;

        return RC_MAP_OK;
      }
    }

    curr = (curr + 1) % map->tableSize;
  }

  return RC_MAP_MISSING;
}

*/





// ===== Value =====







// ===== Data storage =====
#include <vm/datatable.h>

// ===== Exceptions =====
#include <vm/except.h>

// ===== Runtime =====
#include <vm/runtime.h>


// ===== Interpreter =====
#include <vm/interpreter.h>

// ===== Native function arguments =====

value_t *args_getArg(args_t *args, size_t index) {
  return &args->_stack->data[*args->_stack->lenVal - 1 - index];
}


// ===== Builtin bindings =====
value_t _System_createObject(runtime_t *r, args_t *args) {
  return value_createObject(r, r->heap);
}

value_t _System_getObjectMember(runtime_t *r, args_t *args) {
  value_t result;
  result.metadata = TYPE_NONE;

  value_t *target = args_getArg(args, 0);
  value_t *member_key = args_getArg(args, 1);

  char *member_key_str = (char*)value_getRawPointer(member_key);

  if ((target->metadata & (TYPE_POINTER | (FLAG_OBJECT << 8))) != (TYPE_POINTER | (FLAG_OBJECT << 8))) {
    // TODO: throw exception cause its not an object
    return result;
  }

  heap_value_t *hv = value_getHeapNode(target);

  object_t *object = (object_t*)hv->ptr;
  value_t *member_ptr = NULL;

  if (object_getPtr(object, member_key_str, &member_ptr) != OBJECT_OK) {
    // TODO throw exception cause member not found

    return result;
  }

  value_copyValue(r, &result, member_ptr);

  return result;
}

value_t _System_setObjectMember(runtime_t *r, args_t *args) {
  value_t result;
  result.metadata = TYPE_NONE;

  value_t *target = args_getArg(args, 0);
  value_t *member_key = args_getArg(args, 1);
  char *member_key_str = (char*)value_getRawPointer(member_key);

  value_t *member_value = args_getArg(args, 2);

  if ((target->metadata & (TYPE_POINTER | (FLAG_OBJECT << 8))) != (TYPE_POINTER | (FLAG_OBJECT << 8))) {
    // TODO: throw exception cause its not an object
    return result;
  }

  heap_value_t *hv = value_getHeapNode(target);

  object_t *object = (object_t*)hv->ptr;

  int result_code;

  value_copyValue(r, &result, member_value);

  if ((result_code = object_put(object, member_key_str, &result)) != OBJECT_OK) {
    // TODO throw exception cause could not set member
    value_setInt(r, &result, result_code);

    return result;
  }

  return result;
}

// ===== C Lib functions =====

value_t _System_C_exit(runtime_t *r, args_t *args) {
  exit(value_getInt(args_getArg(args, 0)));

  return value_fromRawPointer(NULL, 0);
}

#include <math.h>
value_t _System_C_fmod(runtime_t *r, args_t *args) {
  double a = value_getDouble(args_getArg(args, 0));
  double b = value_getDouble(args_getArg(args, 1));

  return value_fromDouble(fmod(a, b));
}

value_t _System_C_strlen(runtime_t *r, args_t *args) {
  return value_fromInt(strlen((char*)value_getRawPointer(args_getArg(args, 0))));
}

value_t _System_C_fopen(runtime_t *r, args_t *args) {
  char *filename = (char*)value_getRawPointer(args_getArg(args, 0));
  char *mode = (char*)value_getRawPointer(args_getArg(args, 1));

  FILE *file = fopen(filename, mode);

  return value_fromRawPointer((void*)file, 0);
}

value_t _System_C_fclose(runtime_t *r, args_t *args) {
  FILE *file = (FILE*)value_getRawPointer(args_getArg(args, 0));

  return value_fromInt(fclose(file));
}

value_t _System_C_fread(runtime_t *r, args_t *args) {
  FILE *file = (FILE*)value_getRawPointer(args_getArg(args, 0));
  int64_t size = value_getInt(args_getArg(args, 1));

  char *data = malloc(size);

  memset(data, 0, size);

  fread(data, 1, size, file);

  value_t v;
  v.metadata = TYPE_NONE;

  value_setRefCounted(r, &v, data);

  return v;
}

value_t _System_C_fwrite(runtime_t *r, args_t *args) {
  FILE *file = (FILE*)value_getRawPointer(args_getArg(args, 0));
  int64_t size = value_getInt(args_getArg(args, 1));
  void *raw = value_getRawPointer(args_getArg(args, 2));
  size_t result = fwrite(raw, 1, size, file);

  return value_fromInt(result);
}

value_t _System_C_fseek(runtime_t *r, args_t *args) {
  FILE *file = (FILE*)value_getRawPointer(args_getArg(args, 0));
  int64_t offset = value_getInt(args_getArg(args, 1));
  int64_t origin = value_getInt(args_getArg(args, 2));

  return value_fromInt(fseek(file, offset, origin));
}

// ===== Utility functions =====

uint8_t makeInstruction(enum INSTRUCTIONS opcode, uint8_t flags) {
  uint8_t data = opcode;
  data <<= 3;
  data |= flags;
  return data;
}

void showArguments(int argc, char *argv[]) {
  printf("Arguments: %s <filename>\n\t--genc: Generate C source file\n\n", argv[0]);
  exit(EXIT_FAILURE);
}

// ===== threading functions =====

typedef struct {
  runtime_t *rt;
  ubyte_t *data;
  size_t len;
} interpreter_data_t;

void *interpreterThread(void *arg) {
  interpreter_data_t *iData = (interpreter_data_t*)arg;

  interpreter_t *it = interpreter_create(iData->rt, iData->data, iData->len);

  // value_setFunction(iData->rt, datatable_getValue(iData->rt->dt, 0, AT_DATA | AT_ABS), _System_C_exit);

  interpreter_run(it);
  interpreter_destroy(it);

  return NULL;
}

void *gcThread(void *arg) {
  runtime_t *rt = (runtime_t*)arg;

  // @TODO

  return NULL;
}

void openFile(interpreter_data_t *iData, int argc, char *argv[]) {
  FILE *fp = fopen(argv[1], "r");

  if (fp == NULL) {
    puts("Error while opening the file.");
    exit(EXIT_FAILURE);
  }

  /* Go to the end of the file. */
  if (fseek(fp, 0L, SEEK_END) == 0) {
    /* Get the size of the file. */
    long bufSize = ftell(fp);

    if (bufSize == -1) {
      puts("Error reading - bufSize == -1");
      exit(EXIT_FAILURE);
    }

    iData->data = malloc(bufSize);

    if (fseek(fp, 0L, SEEK_SET) != 0) {
      puts("Error reading - could not seek to end");
      exit(EXIT_FAILURE);
    }

    /* Read the entire file into memory. */
    iData->len = fread(iData->data, sizeof(ubyte_t), bufSize, fp);

    if (iData->len != bufSize) {
      printf("Error reading - iData->len (%zu) != bufSize (%ld)", iData->len, bufSize);
      exit(EXIT_FAILURE);
    }

    if (ferror(fp) != 0) {
      puts("Error reading file");
      exit(EXIT_FAILURE);
    }
  }

  fclose(fp);
}

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')



#define SET_BUILTIN_C_FUNCTION(enum_item, c_func) \
  iData.rt->dt->storage[AT_DATA].data[enum_item] = value_fromFunction(c_func)

#include <shared/builtins.h>

#if 1

// ===== Main driver =====
int main(int argc, char *argv[]) {
  MEASURE_EXECUTION_TIME_BEGIN;

  interpreter_data_t iData;

  if (argc == 2 || argc == 3) {
    openFile(&iData, argc, argv);
  } else {
    showArguments(argc, argv);
    return 1;
  }

  iData.rt = runtime_create();
  SET_BUILTIN_C_FUNCTION(BUILTIN_SYSTEM_CREATE_OBJECT, _System_createObject);
  SET_BUILTIN_C_FUNCTION(BUILTIN_SYSTEM_GET_OBJECT_MEMBER, _System_getObjectMember);
  SET_BUILTIN_C_FUNCTION(BUILTIN_SYSTEM_SET_OBJECT_MEMBER, _System_setObjectMember);

  SET_BUILTIN_C_FUNCTION(BUILTIN_SYSTEM_C_EXIT, _System_C_exit);
  SET_BUILTIN_C_FUNCTION(BUILTIN_SYSTEM_C_FMOD, _System_C_fmod);
  SET_BUILTIN_C_FUNCTION(BUILTIN_SYSTEM_C_STRLEN, _System_C_strlen);

  SET_BUILTIN_C_FUNCTION(BUILTIN_SYSTEM_C_FOPEN, _System_C_fopen);
  SET_BUILTIN_C_FUNCTION(BUILTIN_SYSTEM_C_FCLOSE, _System_C_fclose);
  SET_BUILTIN_C_FUNCTION(BUILTIN_SYSTEM_C_FREAD, _System_C_fread);
  SET_BUILTIN_C_FUNCTION(BUILTIN_SYSTEM_C_FWRITE, _System_C_fwrite);
  SET_BUILTIN_C_FUNCTION(BUILTIN_SYSTEM_C_FSEEK, _System_C_fseek);

  bool genc = false;

  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "--genc") == 0) {
      genc = true;
    }
  }

  if (genc) {
    interpreter_t *it = interpreter_create(iData.rt, iData.data, iData.len);
    jit_run(it);
    interpreter_destroy(it);
  } else {
    // execution thread
    pthread_t interpreterThreadId, gcThreadId;

    pthread_create(&interpreterThreadId, NULL, interpreterThread, (void*)&iData);
    pthread_join(interpreterThreadId, NULL);

    pthread_create(&gcThreadId, NULL, gcThread, (void*)iData.rt);
    pthread_join(gcThreadId, NULL);

    runtime_gc(iData.rt);
  }

  runtime_destroy(iData.rt);

  free(iData.data);

  MEASURE_EXECUTION_TIME_END;

  printf("Execution time: %f\n", MEASURE_EXECUTION_TIME_RESULT);

  return 0;
}

#endif
