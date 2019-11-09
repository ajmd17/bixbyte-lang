#pragma once

typedef struct runtime runtime_t;

#include <vm/interpreter.h>

#include <stdint.h>

#define BB8_JIT_BUFFER_SIZE 1024*1024 // 1MB
#define BB8_JIT_CMD_SIZE 1024*8

#define BB8_JIT_APPEND(csrc, ...) \
  snprintf(bb8_cmdStream, BB8_JIT_CMD_SIZE, csrc, __VA_ARGS__)

#define BB8_JIT_HEADER \
  "#define DECODE(loc) datatable_getValue(rt, loc >> 4, loc & 0xF)\n\n" \
  "int main(int argc, char *argv[]) {\n" \
  "runtime_t *rt = runtime_create();\n"

#define BB8_LABEL(pc) \
  snprintf(bb8_cmdStream, BB8_JIT_CMD_SIZE, "\n_lbl_%zu:\n", pcBefore)

static char bb8_jitStream[BB8_JIT_BUFFER_SIZE];

void jit_run(interpreter_t *it, runtime_t *rt) {
  uint8_t data, opcode, flags, cache[64];

  memset(bb8_jitStream, '\0', BB8_JIT_BUFFER_SIZE);
  strcat(bb8_jitStream, BB8_JIT_HEADER);

  while (!interpreter_atEnd(it)) {
    char bb8_cmdStream[BB8_JIT_CMD_SIZE];
    memset(bb8_cmdStream, '\0', BB8_JIT_CMD_SIZE);
    
    size_t pcBefore = it->pc;

    BB8_JIT_APPEND("{\n");

    interpreter_read(it, sizeof(data), &data);

    opcode = data >> 3;
    flags = data & 0x7;

    BB8_LABEL(pcBefore);

    switch (opcode) {
      case OP_NOOP: break;
      case OP_LOAD: { // load
        obj_loc_t o;
        interpreter_read(it, sizeof(o), &o);

        loc_28_t loc;
        archtype_t at;
        obj_loc_parse(o, &loc, &at);

        BB8_JIT_APPEND("value_t *v = DECODE(%#lx);\n", o);

        switch (flags) {
          case CONST_FLAGS_NONE: // ??
            break;
          case CONST_FLAGS_NULL: // loadnull
            BB8_JIT_APPEND("v->data.raw = NULL;\nv->metadata = TYPE_POINTER;\n");
            break;
          case CONST_FLAGS_I64: { // loadi4
            interpreter_read(it, sizeof(int64_t), cache);
            BB8_JIT_APPEND("value_setInt(rt, v, %d);\n", *((int64_t*)cache));

            break;
          }
          case CONST_FLAGS_U64: { // loadu4
            interpreter_read(it, sizeof(uint64_t), cache);
            BB8_JIT_APPEND("value_setUint(rt, v, %d);\n", *((uint64_t*)cache));

            break;
          }
          case CONST_FLAGS_F64: { // loadd
            interpreter_read(it, sizeof(double), cache);
            BB8_JIT_APPEND("value_setDouble(rt, v, %0.f);\n", *((double*)cache));

            break;
          }
          case CONST_FLAGS_BOOL: { // loadb
            interpreter_read(it, sizeof(uint8_t), cache);
            BB8_JIT_APPEND("value_setDouble(rt, v, %d);\n", (bool)cache[0]);

            break;
          }
          case CONST_FLAGS_RAWDATA: { // loaddata
            interpreter_read(it, sizeof(uint64_t), cache);
            uint64_t sz = *((uint64_t*)cache);

            void *data = malloc(sz);
            interpreter_read(it, sz, data);

            BB8_JIT_APPEND("void *data = malloc(%d);\n", sz);
            BB8_JIT_APPEND("static unsigned char _data_%d = { ", it->pc);

            while (sz) {
              BB8_JIT_APPEND("%#lx", ((unsigned char*)data)[sz - 1]);

              if (sz > 1) {
                BB8_JIT_APPEND(", ");
              }

              --sz;
            }

            BB8_JIT_APPEND(" };\n");
            BB8_JIT_APPEND("value_setRefCounted(rt, v, data);\n");

            free(data);

            break;
          }
        }

        break;
      }

      case OP_MOV: { // mov
        obj_loc_t o;

        interpreter_read(it, sizeof(o), &o);

        BB8_JIT_APPEND("value_t *left = DECODE(%#lx);\n", o);

        interpreter_read(it, sizeof(o), &o);

        BB8_JIT_APPEND("value_t *right = DECODE(%#lx);\n", o);

        if (at & AT_REG) {
          BB8_JIT_APPEND("*left = *right;\n");
        } else {
          BB8_JIT_APPEND("value_copyValue(rt, left, right);\n");
        }

        break;
      }

      case OP_CMP: { // cmp
        obj_loc_t left, right;

        interpreter_read(it, sizeof(left), &left);
        interpreter_read(it, sizeof(right), &right);

        BB8_JIT_APPEND("value_t *left = DECODE(%#lx), *right = DECODE(%#lx)", left, right);
        BB8_JIT_APPEND("union { int i; double d; } cacheval;\n");

        switch (flags) {
          case CMP_FLAG_F64_L: // cmpdl
            BB8_JIT_APPEND("cacheval.d = left->data.dbl - right->data.i64;\n"
              "it->flags = ((0 < cacheval.d) - (cacheval.d < 0)) + 1;\n");
            break;
          case CMP_FLAG_F64_R: // cmpdr
            BB8_JIT_APPEND("cacheval.d = left->data.i64 - right->data.dbl;\n"
              "it->flags = ((0 < cacheval.d) - (cacheval.d < 0)) + 1;\n");
            break;
          case CMP_FLAG_F64_LR: // cmpd
            BB8_JIT_APPEND("cacheval.d = left->data.dbl - right->data.dbl;\n"
              "it->flags = ((0 < cacheval.d) - (cacheval.d < 0)) + 1;\n");
            break;
          default: // cmp
            BB8_JIT_APPEND("cacheval.i = left->data.i64 - right->data.i64;\n"
              "it->flags = ((0 < cacheval.i) - (cacheval.i < 0)) + 1;\n");
            break;
        }
 
        break;
      }

      case OP_JMP: { // jmp
        value_t *v;

        obj_loc_t o;
        loc_28_t loc;
        archtype_t at;

        interpreter_read(it, sizeof(o), &o);
        obj_loc_parse(o, &loc, &at);

        BB8_JIT_APPEND("value_t *jmpLoc_%zu = DECODE(%#lx);\n", o);

        char jmpEndLabelName[64] = { '\0' };
        snprintf(jmpEndLabelName, 64, "_jmp_%zu_end", it->pc);

        switch (flags) {
          case JUMP_FLAGS_JE: // je
            BB8_JIT_APPEND("\nif (~it->flags & INTERPRETER_FLAGS_EQUAL) goto %s;\n\n", jmpEndLabelName);
            break;
          case JUMP_FLAGS_JNE: // jne
            BB8_JIT_APPEND("\nif (it->flags & INTERPRETER_FLAGS_EQUAL) goto %s;\n\n", jmpEndLabelName);
            break;
          case JUMP_FLAGS_JG: // jg
            BB8_JIT_APPEND("\nif (~it->flags & INTERPRETER_FLAGS_GREATER) goto %s;\n\n", jmpEndLabelName);
            break;
          case JUMP_FLAGS_JGE: // jge
            BB8_JIT_APPEND("\nif (~it->flags & (INTERPRETER_FLAGS_GREATER | INTERPRETER_FLAGS_EQUAL)) goto %s;\n\n", jmpEndLabelName);
            break;
        }

        BB8_APPEND("\ngoto *((void*)value_getUint(jmpLoc_%zu));\n", o); // Need computed GOTO
        //printf("goto _lbl_%zu;\n", value_getUint(v));
        BB8_APPEND("\n%s:\n", jmpEndLabelName);

        break;
      }

      case OP_PUSH: { // push
        BB8_APPEND("storage_t *stack = &rt->dt->storage[AT_LOCAL];\n");

        switch (flags) {
          case CONST_FLAGS_NONE: { // push -- load value_t to push to stack
            obj_loc_t o;
            loc_28_t loc;
            archtype_t at;

            interpreter_read(it, sizeof(o), &o);
            obj_loc_parse(o, &loc, &at);

            BB8_APPEND("value_copyValue(rt, stack->data[stack->len], DECODE(%#lx));\n", o);

            break;
          }
          // shortcuts for pushing constants directly, rather than using multiple instructions
          // @TODO: make these values be copied from a constant pool, rather than by recreating.
          case CONST_FLAGS_NULL: { // pushnull
            BB8_APPEND("value_t *v = &stack->data[stack->len];\n");
            BB8_APPEND("v->data.raw = NULL;\n");
            BB8_APPEND("v->metadata = TYPE_POINTER;\n");

            break;
          }
          case CONST_FLAGS_I64: { // pushi4
            interpreter_read(it, sizeof(int64_t), cache);
            BB8_APPEND("value_setInt(rt, &stack->data[stack->len], %d);\n", *((int64_t*)cache));

            break;
          }
          case CONST_FLAGS_U64: { // pushu4
            interpreter_read(it, sizeof(uint64_t), cache);
            BB8_APPEND("value_setUint(rt, &stack->data[stack->len], %d);\n", *((uint64_t*)cache));

            break;
          }
          case CONST_FLAGS_F64: { // pushd
            interpreter_read(it, sizeof(double), cache);
            BB8_APPEND("value_setDouble(rt, &stack->data[stack->len], %0.f);\n", *((double*)cache));

            break;
          }
          case CONST_FLAGS_BOOL: { // pushb
            interpreter_read(it, sizeof(uint8_t), cache);
            BB8_APPEND("value_setDouble(rt, &stack->data[stack->len], %d);\n", (bool)cache[0]);

            break;
          }
          case CONST_FLAGS_RAWDATA: { // pushdata
            interpreter_read(it, sizeof(uint64_t), cache);
            uint64_t sz = *((uint64_t*)cache);

            void *data = malloc(sz); // managed by refcounter
            BB8_APPEND("void *data = malloc(%d); /* RC */\n", sz);

            interpreter_read(it, sz, data);

            BB8_APPEND("value_setRefCounted(rt, &stack->data[stack->len], data);\n");

            free(data);

            break;
          }
        }

        BB8_APPEND("++stack->len;\n");

        break;
      }

      case OP_POP: {
        interpreter_read(it, sizeof(uint16_t), cache);
        uint16_t sz = *((uint16_t*)cache);

        //rt->dt->storage[AT_LOCAL].len -= sz;
        BB8_APPEND("storage_t *s = &rt->dt->storage[AT_LOCAL];\n");
        BB8_APPEND("uint16_t sz = %d;\n", sz);
        BB8_APPEND("while (sz--) {\n");
        BB8_APPEND("  value_destroy(rt, &s->data[--s->len]);\n");
        BB8_APPEND("}\n");

        break;
      }

      case OP_ADD:
      case OP_SUB:
      case OP_MUL:
      case OP_DIV:
      case OP_MOD: {
        obj_loc_t o;
        loc_28_t loc;
        archtype_t at;

        interpreter_read(it, sizeof(o), &o);
        obj_loc_parse(o, &loc, &at);

        BB8_APPEND("value_t *left = DECODE(%#lx);\n", o);

        interpreter_read(it, sizeof(o), &o);
        obj_loc_parse(o, &loc, &at);

        BB8_APPEND("value_t *right = DECODE(%#lx);\n", o);

        switch (opcode) {
          case OP_ADD:
            switch (flags) {
              case CMP_FLAG_F64_L:  BB8_APPEND("left->data.dbl = left->data.dbl + right->data.i64;\n"); break;
              case CMP_FLAG_F64_R:  BB8_APPEND("left->data.i64 = left->data.i64 + right->data.dbl;\n"); break;
              case CMP_FLAG_F64_LR: BB8_APPEND("left->data.dbl = left->data.dbl + right->data.dbl;\n"); break;
              default:              BB8_APPEND("left->data.i64 = left->data.i64 + right->data.i64;\n"); break;
            }
            break;
          case OP_SUB:
            switch (flags) {
              case CMP_FLAG_F64_L:  BB8_APPEND("left->data.dbl = left->data.dbl - right->data.i64;\n"); break;
              case CMP_FLAG_F64_R:  BB8_APPEND("left->data.i64 = left->data.i64 - right->data.dbl;\n"); break;
              case CMP_FLAG_F64_LR: BB8_APPEND("left->data.dbl = left->data.dbl - right->data.dbl;\n"); break;
              default:              BB8_APPEND("left->data.i64 = left->data.i64 - right->data.i64;\n"); break;
            }
            break;
          case OP_MUL:
            switch (flags) {
              case CMP_FLAG_F64_L:  BB8_APPEND("left->data.dbl = left->data.dbl * right->data.i64;\n"); break;
              case CMP_FLAG_F64_R:  BB8_APPEND("left->data.i64 = left->data.i64 * right->data.dbl;\n"); break;
              case CMP_FLAG_F64_LR: BB8_APPEND("left->data.dbl = left->data.dbl * right->data.dbl;\n"); break;
              default:              BB8_APPEND("left->data.i64 = left->data.i64 * right->data.i64;\n"); break;
            }
            break;
          // @TODO: div by zero catch?
          case OP_DIV:
            switch (flags) {
              case CMP_FLAG_F64_L:  BB8_APPEND("left->data.dbl = left->data.dbl / right->data.i64;\n"); break;
              case CMP_FLAG_F64_R:  BB8_APPEND("left->data.i64 = left->data.i64 / right->data.dbl;\n"); break;
              case CMP_FLAG_F64_LR: BB8_APPEND("left->data.dbl = left->data.dbl / right->data.dbl;\n"); break;
              default:              BB8_APPEND("left->data.i64 = left->data.i64 / right->data.i64;\n"); break;
            }
            break;
          case OP_MOD:
            BB8_APPEND("left->data.i64 = left->data.i64 % right->data.i64;\n");
            break;
        }

        break;
      }

      case OP_XOR:
      case OP_AND:
      case OP_OR:
      case OP_SHL:
      case OP_SHR: {
        obj_loc_t o;
        loc_28_t loc;
        archtype_t at;

        interpreter_read(it, sizeof(o), &o);
        obj_loc_parse(o, &loc, &at);

        BB8_APPEND("value_t *left = DECODE(%#lx);\n", o);

        interpreter_read(it, sizeof(o), &o);
        obj_loc_parse(o, &loc, &at);

        BB8_APPEND("value_t *right = DECODE(%#lx);\n", o);

        switch (opcode) {
          case OP_XOR: BB8_APPEND("left->data.i64 = left->data.i64 ^ right->data.i64;\n"); break;
          case OP_AND: BB8_APPEND("left->data.i64 = left->data.i64 & right->data.i64;\n"); break;
          case OP_OR:  BB8_APPEND("left->data.i64 = left->data.i64 | right->data.i64;\n"); break;
          case OP_SHL: BB8_APPEND("left->data.i64 = left->data.i64 << right->data.i64;\n"); break;
          case OP_SHR: BB8_APPEND("left->data.i64 = left->data.i64 >> right->data.i64;\n"); break;
        }

        break;
      }

      case OP_NEG: {
        obj_loc_t o;
        loc_28_t loc;
        archtype_t at;

        interpreter_read(it, sizeof(o), &o);
        obj_loc_parse(o, &loc, &at);

        switch (flags) {
          case CMP_FLAG_F64_L:  BB8_APPEND("DECODE(%#lx)->data.dbl *= -1;\n", o); break;
          default:              BB8_APPEND("DECODE(%#lx)->data.i64 *= -1;\n", o); break;
        }

        break;
      }

      case OP_NOT: {
        obj_loc_t o;
        loc_28_t loc;
        archtype_t at;

        interpreter_read(it, sizeof(o), &o);
        obj_loc_parse(o, &loc, &at);

        BB8_APPEND("DECODE(%#lx)->data.i64 = ~DECODE(%#lx)->data.i64;\n", o, o);

        break;
      }

      case OP_CALL: {
        obj_loc_t o;
        loc_28_t loc;
        archtype_t at;

        interpreter_read(it, sizeof(o), &o);
        obj_loc_parse(o, &loc, &at);

        BB8_APPEND("rt->dt->storage[AT_REG].data[0] = value_invoke(rt, DECODE(%#lx));\n", o);

        // @NOTE: reason we are NOT doing value_copyValue() here, is because we want the register value to inherit
        // all responsibilities of `result` here ... including refcounts, free() obligations...

        break;
      }

      // ...

      case OP_JIT: {
        assert(false && "OP_JIT called from within JIT function");

        // move value_t to static data that holds jitFunction

        break;
      }

      case OP_HALT:
        puts("\n\nProgram Terminated\n\n");
        break;
    }

    BB8_APPEND("}\n\n");

    strcat(bb8_jitStream, bb8_cmdStream);    
  }

  strcat(bb8_jitStream, "\nruntime_destroy(rt);\n");
  strcat(bb8_jitStream, "return 0;\n}\n");

  // TODO: run compiler?
  FILE *fptr = fopen("_tmp_jit.c", "w");
  if (fptr == NULL) {
    puts("Failed to open tmp file");
    exit(1);
  }

  fputs(fptr, bb8_jitStream);
  fclose(fptr);
}
