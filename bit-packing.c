#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// ===== Instructions =====

enum ARCHETYPE {
  AT_DATA = 0x1,
  AT_LOCAL = 0x2,
  AT_REG = 0x3,

  AT_REL = 0x8,
  AT_ABS = 0xC
};

typedef uint32_t obj_loc_t;
typedef uint8_t archtype_t;
typedef uint32_t loc_28_t;

obj_loc_t obj_loc_make(loc_28_t loc, archtype_t at) {
  obj_loc_t payload = loc;
  payload <<= 4;
  payload |= at;
  return payload;
}

void obj_loc_parse(obj_loc_t payload, loc_28_t *loc, archtype_t *at) {
  *loc = payload >> 4;
  *at = payload & 0xF;
}

// ===== Values =====

struct value;
typedef struct value value_t;

struct runtime;
typedef struct runtime runtime_t;

typedef struct {
  value_t *argv;
  int argc;
} args_t;



typedef uint32_t metadata_t;
typedef value_t (*native_function_t)(runtime_t*, args_t*);

enum VALUE_TYPE {
  TYPE_NONE = 0x0,
  TYPE_INT = 0x1,
  TYPE_UINT = 0x2,
  TYPE_DOUBLE = 0x3,
  TYPE_BOOLEAN = 0x4,
  TYPE_POINTER = 0x5,
  TYPE_FUNCTION = 0x6
};

enum VALUE_FLAGS {
  FLAG_NONE = 0x0,
  FLAG_MARKED = 0x1,
  FLAG_MANAGED = 0x2,
  FLAG_EXCEPTION = 0x4
};

typedef struct {
  
} heap_node_t;

typedef struct {
  size_t size;
} heap_t;


typedef struct {
  void *ptr;
  // @TODO hashmap of fields?
} object_t;

object_t *object_create() {
  object_t *obj = (object_t*)malloc(sizeof(object_t));
  return obj;
}

void object_destroy(object_t *obj) {
  // @TODO delete fields, call destructor ?

  free(obj);
}

struct value {
  union {
    int64_t i64;
    uint64_t u64;
    double dbl;
    bool b;
    void *ptr;
    native_function_t fn;
  } data;

  metadata_t metadata;
};

void value_setInt(value_t *v, int64_t i64) {
  v->data.i64 = i64;
  v->metadata = TYPE_INT;
}

int64_t value_getInt(value_t *v) {
  return v->data.i64;
}

value_t value_fromInt(int64_t i64) {
  value_t v;
  v.data.i64 = i64;
  v.metadata = TYPE_INT;
  return v;
}

void value_setUint(value_t *v, uint64_t u64) {
  v->data.u64 = u64;
  v->metadata = TYPE_UINT;
}

uint64_t value_getUint(value_t *v) {
  return v->data.u64;
}

value_t value_fromUint(uint64_t u64) {
  value_t v;
  v.data.u64 = u64;
  v.metadata = TYPE_UINT;
  return v;
}

void value_setDouble(value_t *v, double dbl) {
  v->data.dbl = dbl;
  v->metadata = TYPE_DOUBLE;
}

double value_getDouble(value_t *v) {
  return v->data.dbl;
}

value_t value_fromDouble(double dbl) {
  value_t v;
  v.data.dbl = dbl;
  v.metadata = TYPE_DOUBLE;
  return v;
}

void value_setBoolean(value_t *v, bool b) {
  v->data.b = b;
  v->metadata = TYPE_BOOLEAN;
}

bool value_getBool(value_t *v) {
  return v->data.b;
}

value_t value_fromBoolean(bool b) {
  value_t v;
  v.data.b = b;
  v.metadata = TYPE_BOOLEAN;
  return v;
}

value_t value_createObject() {
  value_t v;
  v.data.ptr = (void*)object_create();
  v.metadata = TYPE_POINTER | (FLAG_MANAGED << 8);
  return v;
}

void *value_getPointer(value_t *value) {
  return value->data.ptr;
}

value_t value_fromRawPointer(void *ptr) {
  value_t v;
  v.data.ptr = ptr;
  v.metadata = TYPE_POINTER;
  return v;
}

value_t value_fromFunction(native_function_t fn) {
  value_t v;
  v.data.fn = fn;
  v.metadata = TYPE_FUNCTION;
  return v;
}

VALUE_TYPE value_getType(value_t *value) {
  // type is stored in first 8 bits of metadata
  return (VALUE_TYPE)(value->metadata & 0xFF);
}

void value_setType(value_t *value, VALUE_TYPE type) {
  value->metadata = (value->metadata & 0xFF00) | type;
}

VALUE_FLAGS value_getFlags(value_t *value) {
  // flags stored in next 8 bits of metadata
  return (VALUE_FLAGS)(value->metadata & 0xFF00);
}

void value_setFlag(value_t *value, VALUE_FLAGS flag, int state) {
  value->metadata = (-state & (value->metadata | (flag << 8))) | ((state - 1) & (value->metadata & ~(flag << 8)));
}

uintptr_t value_getID(value_t *value) {
  switch (value_getType(value)) {
    case TYPE_POINTER:
      return (uintptr_t)value->data.ptr;
    case TYPE_FUNCTION:
      return (uintptr_t)value->data.fn;
    default:
      return (uintptr_t)(&value->data);
  }
  //int isptr = !!(value->metadata & TYPE_POINTER);
  //return (-isptr & (uintptr_t)value->data.ptr) | ((isptr - 1) & (uintptr_t)(&value->data));
}

value_t value_invoke(runtime_t *r, value_t *value, int argc, value_t *argv) {
  args_t args;
  args.argv = argv;
  args.argc = argc;

  return value->data.fn(r, &args);
}

void value_destroy(value_t *value) {
  if (value->metadata & (TYPE_POINTER | (FLAG_MANAGED << 8))) {
    object_destroy((object_t*)value->data.ptr);
  }
}

// ===== Data storage =====

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

void datatable_destroy(datatable_t *dt) {
  int i;

  for (i = 0; i < 4; i++) {
    while (dt->storage[i].len) {
      value_destroy(&dt->storage[i].data[--dt->storage[i].len]);
    }

    free(dt->storage[i].data);
  }
  
  free(dt);
}

value_t *datatable_getValue(datatable_t *dt, loc_28_t loc, archtype_t at) {
  storage_t *s = &dt->storage[at & 0x3];

  int abs = ((at & 0xC) >> 2) - 2;
  size_t idx = (-abs & loc) | ((abs - 1) & (s->len - loc));

  return &s->data[idx];
}

// ===== Exceptions =====

typedef struct {
  object_t *base;
  value_t argument;
} exception_t;

exception_t exception_fromValue(value_t *argument) {
  exception_t e;
  e.base = NULL; // @TODO SimpleException
  e.argument = *argument;
  return e;
}

// ===== Runtime =====
struct runtime {
  datatable_t *dt;
};

runtime_t *runtime_create() {
  runtime_t *rt = (runtime_t*)malloc(sizeof(runtime_t));

  rt->dt = datatable_create();

  return rt;
}

void runtime_destroy(runtime_t *rt) {
  datatable_destroy(rt->dt);
  free(rt);
}

value_t runtime_throwException(runtime_t *r, exception_t *e) {
  // @TODO internal VM handling.

  value_t value = value_fromRawPointer((void*)e);
  value_setFlag(&value, FLAG_EXCEPTION, 1);
  
  return value;
}

// ===== Interpreter =====
typedef uint8_t ubyte_t;

struct {
  size_t pc;
  size_t len;
  ubyte_t *bc;
} interpreter_t;

interpreter_t *interpreter_create(ubyte_t *data, size_t len) {
  interpreter_t *it = (interpreter_t*)malloc(sizeof(interpreter_t));
  it->pc = 0;
  it->len = len;
  it->bc = malloc(sizeof(ubyte_t) * len);
  memcpy(it->bc, data, it->len);
  return it;
}

void interpreter_destroy(interpreter_t *it) {
  free(it->bc);
  free(it);
}

void interpreter_peek(interpreter_t *it, size_t size, void *out) {
  memcpy(out, it->bc + it->pc, size);
}

void interpreter_read(interpreter_t *it, size_t size, void *out) {
  // assert(it->pc + n <= it->len);
  interpreter_peek(it, size, out);
  it->pc += size;
}

bool interpreter_atEnd(interpreter *it) {
  return it->pc >= it->len;
}

enum JUMPMODE {
  JUMPMODE_JE,
  JUMPMODE_JNE,
  JUMPMODE_JG,
  JUMPMODE_JGE
};

enum LOAD_CONST_FLAGS {
  CONST_TYPE_NULL,
  CONST_TYPE_I64,
  CONST_TYPE_U64,
  CONST_TYPE_F64,
  CONST_TYPE_BOOL,
  // ...
};

static size_t[] LOAD_CONST_SIZES = { 0, sizeof(int64_t), sizeof(uint64_t), sizeof(double), sizeof(bool) };

// instruction = 29 bits
// flags = 3 bits
enum INSTRUCTIONS {
  NOOP = 0x0,

  LOAD_CONST, // load_const | (int, )
  
  MOV,
  
  JMP, // jmp [value: u32]
  JMP_COND, // jmp_cond [value: u32]
  
  PUSH, // push [value: u32]
  POP, // pop [x: u8]
};

void interpreter_loop(interpreter_t *it) {
  while (!interpreter_atEnd(it)) {
  	uint32_t data;

  	interpreter_read(it, sizeof(data), &data);

	uint32_t opcode = data >> 3;
	uint8_t flags = data & 0x7;

	switch (opcode) {
	  case NOOP: break;
	  case LOAD_CONST:
	    
	}
  }
}

// ===== C Lib functions =====

#include <math.h>
value_t _System_C_fmod(runtime_t *r, args_t *args) {
  if (args->argc != 2) {
  	return runtime_throwException(r, NULL);
  }

  return value_fromDouble(fmod(value_getDouble(&args->argv[0]), value_getDouble(&args->argv[1])));
}

value_t _System_C_strlen(runtime_t *r, args_t *args) {
  if (args->argc != 1) {
  	return runtime_throwException(r, NULL);
  }

  return value_fromInt(strlen((char*)value_getPointer(&args->argv[0])));
}

// ===== Main driver =====
int main(int argc, char *argv[]) {
  obj_loc_t o = obj_loc_make(123, AT_DATA | AT_ABS);

  loc_28_t loc;
  archtype_t at;
  obj_loc_parse(o, &loc, &at);
  // =====
  
  runtime_t *rt = runtime_create();
  
  {
	  value_t fmod_func = value_fromFunction(_System_C_fmod);
	  value_t fmod_args[2];
	  fmod_args[0] = value_fromDouble(123);
	  fmod_args[1] = value_fromDouble(456);
	  value_t result = value_invoke(rt, &fmod_func, 2, fmod_args);
	  printf("%f\n", value_getDouble(&result));
  }
  
  {
	  value_t strlen_func = value_fromFunction(_System_C_strlen);
	  value_t strlen_args[1];
	  strlen_args[0] = value_fromRawPointer((void*)"Hello World");
	  value_t result = value_invoke(rt, &strlen_func, 1, strlen_args);
	  printf("%d\n", value_getInt(&result));
  }
  
  //value_setFlag(v, FLAG_MANAGED, 1);
  //std::cout << "1: " << (std::bitset<32>(v->metadata)) << std::endl;
  
  //value_setFlag(v, FLAG_MARKED, 1);
  //std::cout << "2: " << (std::bitset<32>(v->metadata)) << std::endl;
  //value_setFlag(v, FLAG_MANAGED, 0);
  //std::cout << "3: " << (std::bitset<32>(v->metadata)) << std::endl;
  
  
  runtime_destroy(rt);

  

  return 0;
}
