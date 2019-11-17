#pragma once

#include <vm/runtime.h>

#include <stdint.h>
#include <stddef.h>

typedef uint8_t ubyte_t;
typedef struct interpreter interpreter_t;

struct interpreter {
  size_t pc;
  size_t len;
  uint8_t flags;
  ubyte_t *bc;
  runtime_t *rt;
};

typedef enum {
  INTERPRETER_FLAGS_NONE = 0x0,
  INTERPRETER_FLAGS_EQUAL = 0x1,
  INTERPRETER_FLAGS_GREATER = 0x2
} INTERPRETER_FLAGS;

enum JUMP_FLAGS {
  JUMP_FLAGS_NONE = 0x0,
  JUMP_FLAGS_JE = 0x1,
  JUMP_FLAGS_JNE = 0x2,
  JUMP_FLAGS_JG = 0x3,
  JUMP_FLAGS_JGE = 0x4
};

enum CONST_FLAGS {
  CONST_FLAGS_NONE = 0x0,
  CONST_FLAGS_NULL = 0x1,
  CONST_FLAGS_I64 = 0x2,
  CONST_FLAGS_U64 = 0x3,
  CONST_FLAGS_F64 = 0x4,
  CONST_FLAGS_BOOL = 0x5,
  // empty
  CONST_FLAGS_RAWDATA = 0x7
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

  OP_CALL = 20,
  OP_PRINT = 21,
  OP_PLACEHOLDER_22 = 22,
  OP_PLACEHOLDER_23 = 23,
  OP_PLACEHOLDER_24 = 24,
  OP_PLACEHOLDER_25 = 25,
  OP_PLACEHOLDER_26 = 26,
  OP_PLACEHOLDER_27 = 27,
  OP_PLACEHOLDER_28 = 28,
  OP_PLACEHOLDER_29 = 29,
  OP_JIT = 30,

  OP_HALT = 31, // exit program
};

interpreter_t *interpreter_create(runtime_t *rt, ubyte_t *data, size_t len);
void interpreter_destroy(interpreter_t *it);
void interpreter_peek(interpreter_t *it, size_t size, void *out);
void interpreter_seek(interpreter_t *it, size_t loc);
void interpreter_read(interpreter_t *it, size_t size, void *out);
bool interpreter_atEnd(interpreter_t *it);

void interpreter_run(interpreter_t *it);
