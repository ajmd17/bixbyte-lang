#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

// ===== Instructions =====

typedef enum {
  AT_DATA = 0x1,
  AT_LOCAL = 0x2,
  AT_REG = 0x3,

  AT_REL = 0x8,
  AT_ABS = 0xC
} ARCHETYPE;

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

typedef enum {
  TYPE_NONE = 0x0,
  TYPE_INT = 0x1,
  TYPE_UINT = 0x2,
  TYPE_DOUBLE = 0x3,
  TYPE_BOOLEAN = 0x4,
  TYPE_POINTER = 0x5,
  TYPE_FUNCTION = 0x6
} VALUE_TYPE;

typedef enum {
  FLAG_NONE = 0x0,
  FLAG_MARKED = 0x1,
  FLAG_OBJECT = 0x2,
  FLAG_EXCEPTION = 0x4,
  FLAG_MANAGED = 0x8, // raw pointer that needs free() call
} VALUE_FLAGS;

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

void value_copyValue(value_t *v, value_t *other) {
  v->data = other->data;
  v->metadata = other->metadata;
}

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

bool value_getBoolean(value_t *v) {
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
  v.metadata = TYPE_POINTER | (FLAG_OBJECT << 8);
  return v;
}

void *value_getPointer(value_t *value) {
  return value->data.ptr;
}

void value_setRawPointer(value_t *v, void *ptr, VALUE_FLAGS flags) {
  v->data.ptr = ptr;
  v->metadata = TYPE_POINTER | (flags << 8);
}

value_t value_fromRawPointer(void *ptr, VALUE_FLAGS flags) {
  value_t v;
  v.data.ptr = ptr;
  v.metadata = TYPE_POINTER | (flags << 8);
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
  if (value->metadata & (TYPE_POINTER | (FLAG_OBJECT << 8))) {
    object_destroy((object_t*)value->data.ptr);
  } else if (value->metadata & (TYPE_POINTER | (FLAG_MANAGED << 8))) {
    free(value->data.ptr);
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

void datatable_setValue(datatable_t *dt, loc_28_t loc, archtype_t at, value_t *v) {
  storage_t *s = &dt->storage[at & 0x3];

  int abs = ((at & 0xC) >> 2) - 2;
  size_t idx = (-abs & loc) | ((abs - 1) & (s->len - loc));

  s->data[idx] = *v;
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

  value_t value = value_fromRawPointer((void*)e, FLAG_EXCEPTION);
  
  return value;
}

// ===== Interpreter =====
typedef uint8_t ubyte_t;

typedef struct {
  size_t pc;
  size_t len;
  uint8_t flags;
  ubyte_t *bc;
} interpreter_t;

typedef enum {
  INTERPRETER_FLAGS_NONE = 0x0,
  INTERPRETER_FLAGS_EQUAL = 0x1,
  INTERPRETER_FLAGS_GREATER = 0x2
} INTERPRETER_FLAGS;

interpreter_t *interpreter_create(ubyte_t *data, size_t len) {
  interpreter_t *it = (interpreter_t*)malloc(sizeof(interpreter_t));
  it->pc = 0;
  it->len = len;
  it->flags = 0;
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

void interpreter_seek(interpreter_t *it, size_t loc) {
  // assert(loc < it->len);
  it->pc = loc;
}

void interpreter_read(interpreter_t *it, size_t size, void *out) {
  // assert(it->pc + n <= it->len);
  interpreter_peek(it, size, out);
  it->pc += size;
}

bool interpreter_atEnd(interpreter_t *it) {
  return it->pc >= it->len;
}

enum JUMP_FLAGS {
  JUMP_FLAGS_NONE = 0x0,
  JUMP_FLAGS_JE = 0x1,
  JUMP_FLAGS_JNE = 0x2,
  JUMP_FLAGS_JG = 0x3,
  JUMP_FLAGS_JGE = 0x4
};

enum LOAD_CONST_FLAGS {
  LOAD_FLAGS_NULL = 0,
  LOAD_FLAGS_I64 = 1,
  LOAD_FLAGS_U64 = 2,
  LOAD_FLAGS_F64 = 3,
  LOAD_FLAGS_BOOL = 4,

  LOAD_FLAGS_PLACEHOLDER5 = 5,
  LOAD_FLAGS_PLACEHOLDER6 = 6,

  LOAD_FLAGS_RAWDATA = 7
};

enum CMP_FLAG {
  CMP_FLAG_NONE = 0x0, // normal `cmp` -- compares .data to .data
  CMP_FLAG_F64_L = 0x1, // compare .data.f64 to .data
  CMP_FLAG_F64_R = 0x2, // compare .data to .data.f64
  CMP_FLAG_F64_LR = 0x3, // compare .data.f64 to .data.f64
};

//static size_t LOAD_CONST_SIZES[] = { 0, sizeof(int64_t), sizeof(uint64_t), sizeof(double), sizeof(bool) };

// instruction = 5 bits
// flags = 3 bits
enum INSTRUCTIONS { // max: 32 values
  OP_NOOP = 0,

  OP_LOAD = 1, // load value into register

  OP_MOV = 2, // move data from one place to another

  OP_CMP = 4, // compare and set compare flag

  OP_JMP = 5, // jump to location held in value
  
  OP_PUSH = 6, // push value to stack
  OP_POP = 7, // pop n values from stack 

  // ===== binary operations
  OP_ADD = 8, // +
  OP_SUB = 9, // -
  OP_MUL = 10, // *
  OP_DIV = 11, // /
  OP_MOD = 12, // %

  // ===== bitwise (binary) operations
  OP_XOR = 13, // ^
  OP_AND = 14, // &
  OP_OR = 15, // |
  OP_SHL = 16, // <<
  OP_SHR = 17, // >>

  // ===== unary operations
  OP_NEG = 18, // - (mathematical negation)
  OP_NOT = 19, // ~

  OP_PLACEHOLDER_20 = 20, // cast ?
  OP_PLACEHOLDER_21 = 21,
  OP_PLACEHOLDER_22 = 22,
  OP_PLACEHOLDER_23 = 23,
  OP_PLACEHOLDER_24 = 24,
  OP_PLACEHOLDER_25 = 25,
  OP_PLACEHOLDER_26 = 26,
  OP_PLACEHOLDER_27 = 27,
  OP_PLACEHOLDER_28 = 28,
  OP_PLACEHOLDER_29 = 29,
  OP_PLACEHOLDER_30 = 30,

  OP_HALT = 31, // exit program
};

void interpreter_loop(interpreter_t *it) {
  uint8_t data, opcode, flags, cache[64];

  runtime_t *rt = runtime_create();

  // uint8_t *const_data_pool[8];
  // const_data_pool[LOAD_FLAGS_NULL] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x5 };
  // const_data_pool[LOAD_FLAGS_I64] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1 };
  // const_data_pool[LOAD_FLAGS_U64] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2 };
  // const_data_pool[LOAD_FLAGS_F64] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3 };
  // const_data_pool[LOAD_FLAGS_BOOL] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4 };
  // const_data_pool[LOAD_FLAGS_RAWDATA] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4 };

  while (!interpreter_atEnd(it)) {
    interpreter_read(it, sizeof(data), &data);

    opcode = data >> 3;
    flags = data & 0x7;

    switch (opcode) {
      case OP_NOOP: break;
      case OP_LOAD: {
        obj_loc_t o;
        interpreter_read(it, sizeof(o), &o);

        loc_28_t loc;
        archtype_t at;
        obj_loc_parse(o, &loc, &at);

        value_t *v = datatable_getValue(rt->dt, loc, at);

        switch (flags) {
          case LOAD_FLAGS_NULL:
            v->data.ptr = NULL;
            v->metadata = TYPE_POINTER;

            break;
          case LOAD_FLAGS_I64: {
            interpreter_read(it, sizeof(int64_t), cache);
            value_setInt(v, *((int64_t*)cache));

            break;
          }
          case LOAD_FLAGS_U64: {
            interpreter_read(it, sizeof(uint64_t), cache);
            value_setUint(v, *((uint64_t*)cache));

            break;
          }
          case LOAD_FLAGS_F64: {
            interpreter_read(it, sizeof(double), cache);
            value_setDouble(v, *((double*)cache));

            break;
          }
          case LOAD_FLAGS_BOOL: {
            interpreter_read(it, sizeof(uint8_t), cache);
            value_setBoolean(v, (bool)cache[0]);

            break;
          }
          case LOAD_FLAGS_RAWDATA: {
            interpreter_read(it, sizeof(uint64_t), cache);
            uint64_t sz = *((uint64_t*)cache);

            void *data = malloc(sz);
            interpreter_read(it, sz, data);

            value_setRawPointer(v, data, FLAG_MANAGED);

            break;
          }
        }

        break;
      }

      case OP_MOV: {
        value_t *left, *right;

        obj_loc_t o;
        loc_28_t loc;
        archtype_t at;

        interpreter_read(it, sizeof(o), &o);
        obj_loc_parse(o, &loc, &at);

        left = datatable_getValue(rt->dt, loc, at);

        interpreter_read(it, sizeof(o), &o);
        obj_loc_parse(o, &loc, &at);

        right = datatable_getValue(rt->dt, loc, at);

        value_copyValue(left, right);

        break;
      }

      case OP_CMP: {
        value_t *left, *right;

        obj_loc_t o;
        loc_28_t loc;
        archtype_t at;

        interpreter_read(it, sizeof(o), &o);
        obj_loc_parse(o, &loc, &at);

        left = datatable_getValue(rt->dt, loc, at);

        interpreter_read(it, sizeof(o), &o);
        obj_loc_parse(o, &loc, &at);

        right = datatable_getValue(rt->dt, loc, at);

        union { int i; double d; } cacheval;

        switch (flags) {
          case CMP_FLAG_F64_L:
            cacheval.d = left->data.dbl - right->data.i64;
            goto setDouble;
          case CMP_FLAG_F64_R:
            cacheval.d = left->data.i64 - right->data.dbl;
            goto setDouble;
          case CMP_FLAG_F64_LR:
            cacheval.d = left->data.dbl - right->data.dbl;
            goto setDouble;
          default:
            cacheval.i = left->data.i64 - right->data.i64;
            goto setInt;
        }

      setDouble:
        it->flags = ((0 < cacheval.d) - (cacheval.d < 0)) + 1;
        break;
      setInt:
        it->flags = ((0 < cacheval.i) - (cacheval.i < 0)) + 1; // 0, 1, or 2. gets sign() of `val` and adds 1.
        break;
      }

      case OP_JMP: {
        value_t *v;

        obj_loc_t o;
        loc_28_t loc;
        archtype_t at;

        interpreter_read(it, sizeof(o), &o);
        obj_loc_parse(o, &loc, &at);

        v = datatable_getValue(rt->dt, loc, at);

        switch (flags) {
          case JUMP_FLAGS_JE:
            if (~it->flags & INTERPRETER_FLAGS_EQUAL) {
              goto noSeek;
            }
            break;
          case JUMP_FLAGS_JNE:
            if (it->flags & INTERPRETER_FLAGS_EQUAL) {
              goto noSeek;
            }
            break;
          case JUMP_FLAGS_JG:
            if (~it->flags & INTERPRETER_FLAGS_GREATER) {
              goto noSeek;
            }
            break;
          case JUMP_FLAGS_JGE:
            if (~it->flags & (INTERPRETER_FLAGS_GREATER | INTERPRETER_FLAGS_EQUAL)) {
              goto noSeek;
            }
            break;
        }

        interpreter_seek(it, value_getUint(v));
      noSeek:
        break;
      }

      case OP_PUSH: {
        storage_t stack = rt->dt->storage[AT_LOCAL];

        obj_loc_t o;
        loc_28_t loc;
        archtype_t at;

        interpreter_read(it, sizeof(o), &o);
        obj_loc_parse(o, &loc, &at);

        value_copyValue(&stack.data[stack.len++], datatable_getValue(rt->dt, loc, at));

        break;
      }

      case OP_POP: {
        // assert(stack.len != 0);
        // if ref counting ... value_destroy(stack[stack.len--]);
        interpreter_read(it, sizeof(uint16_t), cache);
        uint16_t sz = *((uint16_t*)cache);

        rt->dt->storage[AT_LOCAL].len -= sz;

        break;
      }

      case OP_ADD:
      case OP_SUB:
      case OP_MUL:
      case OP_DIV:
      case OP_MOD: {
        value_t *left, *right;

        obj_loc_t o;
        loc_28_t loc;
        archtype_t at;

        interpreter_read(it, sizeof(o), &o);
        obj_loc_parse(o, &loc, &at);

        left = datatable_getValue(rt->dt, loc, at);

        interpreter_read(it, sizeof(o), &o);
        obj_loc_parse(o, &loc, &at);

        right = datatable_getValue(rt->dt, loc, at);

        switch (opcode) {
          case OP_ADD:
            switch (flags) {
              case CMP_FLAG_F64_L:  left->data.dbl = left->data.dbl + right->data.i64; break;
              case CMP_FLAG_F64_R:  left->data.i64 = left->data.i64 + right->data.dbl; break;
              case CMP_FLAG_F64_LR: left->data.dbl = left->data.dbl + right->data.dbl; break;
              default:              left->data.i64 = left->data.i64 + right->data.i64; break;
            }
            break;
          case OP_SUB:
            switch (flags) {
              case CMP_FLAG_F64_L:  left->data.dbl = left->data.dbl - right->data.i64; break;
              case CMP_FLAG_F64_R:  left->data.i64 = left->data.i64 - right->data.dbl; break;
              case CMP_FLAG_F64_LR: left->data.dbl = left->data.dbl - right->data.dbl; break;
              default:              left->data.i64 = left->data.i64 - right->data.i64; break;
            }
            break;
          case OP_MUL:
            switch (flags) {
              case CMP_FLAG_F64_L:  left->data.dbl = left->data.dbl * right->data.i64; break;
              case CMP_FLAG_F64_R:  left->data.i64 = left->data.i64 * right->data.dbl; break;
              case CMP_FLAG_F64_LR: left->data.dbl = left->data.dbl * right->data.dbl; break;
              default:              left->data.i64 = left->data.i64 * right->data.i64; break;
            }
            break;
          // @TODO: div by zero catch?
          case OP_DIV:
            switch (flags) {
              case CMP_FLAG_F64_L:  left->data.dbl = left->data.dbl / right->data.i64; break;
              case CMP_FLAG_F64_R:  left->data.i64 = left->data.i64 / right->data.dbl; break;
              case CMP_FLAG_F64_LR: left->data.dbl = left->data.dbl / right->data.dbl; break;
              default:              left->data.i64 = left->data.i64 / right->data.i64; break;
            }
            break;
          case OP_MOD:
            left->data.i64 = left->data.i64 % right->data.i64;
            break;
        }

        break;
      }

      case OP_XOR:
      case OP_AND:
      case OP_OR:
      case OP_SHL:
      case OP_SHR: {
        value_t *left, *right;

        obj_loc_t o;
        loc_28_t loc;
        archtype_t at;

        interpreter_read(it, sizeof(o), &o);
        obj_loc_parse(o, &loc, &at);

        left = datatable_getValue(rt->dt, loc, at);

        interpreter_read(it, sizeof(o), &o);
        obj_loc_parse(o, &loc, &at);

        right = datatable_getValue(rt->dt, loc, at);

        switch (opcode) {
          case OP_XOR: left->data.i64 = left->data.i64 ^ right->data.i64; break;
          case OP_AND: left->data.i64 = left->data.i64 & right->data.i64; break;
          case OP_OR:  left->data.i64 = left->data.i64 | right->data.i64; break;
          case OP_SHL: left->data.i64 = left->data.i64 << right->data.i64; break;
          case OP_SHR: left->data.i64 = left->data.i64 >> right->data.i64; break;
        }

        break;
      }

      case OP_NEG: {
        value_t *left;

        obj_loc_t o;
        loc_28_t loc;
        archtype_t at;

        interpreter_read(it, sizeof(o), &o);
        obj_loc_parse(o, &loc, &at);

        left = datatable_getValue(rt->dt, loc, at);

        switch (flags) {
          case CMP_FLAG_F64_L:  left->data.dbl = -left->data.dbl; break;
          default:              left->data.i64 = -left->data.i64; break;
        }

        break;
      }

      case OP_NOT: {
        value_t *left;

        obj_loc_t o;
        loc_28_t loc;
        archtype_t at;

        interpreter_read(it, sizeof(o), &o);
        obj_loc_parse(o, &loc, &at);

        left = datatable_getValue(rt->dt, loc, at);
        left->data.i64 = ~left->data.i64;

        break;
      }

      // ...

      case OP_HALT:
        puts("\n\nProgram Terminated\n\n");
        break;
    }
  }

  runtime_destroy(rt);
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


uint8_t makeInstruction(enum INSTRUCTIONS opcode, uint8_t flags) {
  uint8_t data = opcode;
  data <<= 3;
  data |= flags;
  return data;
}

void showArguments(int argc, char *argv[]) {
  printf("Arguments: %s <filename>\n", argv[0]);
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
  if (argc == 2) {
    showArguments(argc, argv);
  }

  ubyte_t *data;
  size_t len;

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

    data = malloc(bufSize);

    if (fseek(fp, 0L, SEEK_SET) != 0) {
      puts("Error reading - could not seek to end");
      exit(EXIT_FAILURE);
    }

    /* Read the entire file into memory. */
    size_t readAmt = fread(data, sizeof(ubyte_t), bufSize, fp);

    if (readAmt != bufSize) {
      printf("Error reading - readAmt (%zu) != bufSize (%ld)", readAmt, bufSize);
      exit(EXIT_FAILURE);
    }

    if (ferror(fp) != 0) {
      puts("Error reading file");
      exit(EXIT_FAILURE);
    }
  }

  fclose(fp);

  interpreter_t *it = interpreter_create(data, len);

  interpreter_loop(it);
  interpreter_destroy(it);

  free(data);

  return 0;
}
