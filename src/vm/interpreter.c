#include <vm/interpreter.h>

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

interpreter_t *interpreter_create(runtime_t *rt, ubyte_t *data, size_t len) {
  interpreter_t *it = (interpreter_t*)malloc(sizeof(interpreter_t));
  it->pc = 0;
  it->len = len;
  it->flags = 0;

  it->rt = rt;

  it->bc = malloc(sizeof(ubyte_t) * len);
  memcpy(it->bc, data, it->len);

  return it;
}

void interpreter_destroy(interpreter_t *it) {
  free(it->bc);
  free(it);
}

void interpreter_peek(interpreter_t *it, size_t size, void *out) {
  size_t pc = VM_PROGRAM_COUNTER(it->rt->dt);

  memcpy(out, it->bc + pc, size);
}

void interpreter_seek(interpreter_t *it, size_t loc) {
  // assert(loc < it->len);
  VM_PROGRAM_COUNTER(it->rt->dt) = loc;
}

void interpreter_read(interpreter_t *it, size_t size, void *out) {
  // assert(it->pc + n <= it->len);
  interpreter_peek(it, size, out);
  VM_PROGRAM_COUNTER(it->rt->dt) += size;
}

bool interpreter_atEnd(interpreter_t *it) {
  return VM_PROGRAM_COUNTER(it->rt->dt) >= it->len;
}

void interpreter_run(interpreter_t *it) {
  runtime_t *rt = it->rt;

  uint8_t data, opcode, flags, cache[64];

  while (!interpreter_atEnd(it)) {
    size_t pcBefore = VM_PROGRAM_COUNTER(rt->dt);

    interpreter_read(it, sizeof(data), &data);

    opcode = data >> 3;
    flags = data & 0x7;

    // printf("stack len : %zu\n", VM_STACK_POINTER(rt->dt));

    switch (opcode) {
      case OP_NOOP: break;
      case OP_LOAD: { // load
        obj_loc_t o;
        interpreter_read(it, sizeof(o), &o);

        loc_28_t loc;
        archtype_t at;
        obj_loc_parse(o, &loc, &at);

        value_t *v = datatable_getValue(rt->dt, loc, at);

        switch (flags) {
          case CONST_FLAGS_NONE: // ??
            v->data.i64 = 0;
            v->metadata = TYPE_NONE; // just zero out i guess

            break;
          case CONST_FLAGS_NULL: // loadnull
            v->data.raw = NULL;
            v->metadata = TYPE_POINTER;

            break;
          case CONST_FLAGS_I64: { // loadi4
            interpreter_read(it, sizeof(int64_t), cache);
            value_setInt(rt, v, *((int64_t*)cache));

            break;
          }
          case CONST_FLAGS_U64: { // loadu4
            interpreter_read(it, sizeof(uint64_t), cache);
            value_setUint(rt, v, *((uint64_t*)cache));

            break;
          }
          case CONST_FLAGS_F64: { // loadd
            interpreter_read(it, sizeof(double), cache);
            value_setDouble(rt, v, *((double*)cache));

            break;
          }
          case CONST_FLAGS_BOOL: { // loadb
            interpreter_read(it, sizeof(uint8_t), cache);
            value_setBoolean(rt, v, (bool)cache[0]);

            break;
          }
          case CONST_FLAGS_RAWDATA: { // loaddata
            interpreter_read(it, sizeof(uint64_t), cache);
            uint64_t sz = *((uint64_t*)cache);

            void *data = malloc(sz); // managed by refcounter

            interpreter_read(it, sz, data);

            value_setRefCounted(rt, v, data);

            break;
          }
        }

        break;
      }

      case OP_MOV: { // mov
        value_t *left, *right;

        obj_loc_t o;
        loc_28_t locLeft;
        archtype_t atLeft;
        loc_28_t locRight;
        archtype_t atRight;

        interpreter_read(it, sizeof(o), &o);
        obj_loc_parse(o, &locLeft, &atLeft);

        left = datatable_getValue(rt->dt, locLeft, atLeft);

        interpreter_read(it, sizeof(o), &o);
        obj_loc_parse(o, &locRight, &atRight);

        right = datatable_getValue(rt->dt, locRight, atRight);

        if (atLeft & AT_REG) {
          // optimization
          *left = *right;
        } else {
          value_copyValue(rt, left, right);
        }

        break;
      }

      case OP_CMP: { // cmp
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
          case CMP_FLAG_F64_L: // cmpdl
            cacheval.d = left->data.dbl - right->data.i64;
            goto setDouble;
          case CMP_FLAG_F64_R: // cmpdr
            cacheval.d = left->data.i64 - right->data.dbl;
            goto setDouble;
          case CMP_FLAG_F64_LR: // cmpd
            cacheval.d = left->data.dbl - right->data.dbl;
            goto setDouble;
          default: // cmp
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

      case OP_JMP: { // jmp
        value_t *v;

        obj_loc_t o;
        loc_28_t loc;
        archtype_t at;

        interpreter_read(it, sizeof(o), &o);
        obj_loc_parse(o, &loc, &at);

        v = datatable_getValue(rt->dt, loc, at);

        switch (flags) {
          case JUMP_FLAGS_JE: // je
            if (~it->flags & INTERPRETER_FLAGS_EQUAL) {
              goto noSeek;
            }
            break;
          case JUMP_FLAGS_JNE: // jne
            if (it->flags & INTERPRETER_FLAGS_EQUAL) {
              goto noSeek;
            }
            break;
          case JUMP_FLAGS_JG: // jg
            if (~it->flags & INTERPRETER_FLAGS_GREATER) {
              goto noSeek;
            }
            break;
          case JUMP_FLAGS_JGE: // jge
            if (~it->flags & (INTERPRETER_FLAGS_GREATER | INTERPRETER_FLAGS_EQUAL)) {
              goto noSeek;
            }
            break;
        }

        interpreter_seek(it, value_getUint(v));

      noSeek:
        break;
      }

      case OP_PUSH: { // push
        storage_t *stack = &rt->dt->storage[AT_LOCAL];
        size_t stackLen = *stack->lenVal;

        switch (flags) {
          case CONST_FLAGS_NONE: { // push -- load value_t to push to stack
            obj_loc_t o;
            loc_28_t loc;
            archtype_t at;

            interpreter_read(it, sizeof(o), &o);
            obj_loc_parse(o, &loc, &at);

            value_copyValue(rt, &stack->data[stackLen], datatable_getValue(rt->dt, loc, at));

            break;
          }
          // shortcuts for pushing constants directly, rather than using multiple instructions
          // @TODO: make these values be copied from a constant pool, rather than by recreating.
          case CONST_FLAGS_NULL: { // pushnull
            value_t *v = &stack->data[stackLen];
            v->data.raw = NULL;
            v->metadata = TYPE_POINTER;

            break;
          }
          case CONST_FLAGS_I64: { // pushi4
            interpreter_read(it, sizeof(int64_t), cache);
            value_setInt(rt, &stack->data[stackLen], *((int64_t*)cache));

            break;
          }
          case CONST_FLAGS_U64: { // pushu4
            interpreter_read(it, sizeof(uint64_t), cache);
            value_setUint(rt, &stack->data[stackLen], *((uint64_t*)cache));

            break;
          }
          case CONST_FLAGS_F64: { // pushd
            interpreter_read(it, sizeof(double), cache);
            value_setDouble(rt, &stack->data[stackLen], *((double*)cache));

            break;
          }
          case CONST_FLAGS_BOOL: { // pushb
            interpreter_read(it, sizeof(uint8_t), cache);
            value_setBoolean(rt, &stack->data[stackLen], (bool)cache[0]);

            break;
          }
          case CONST_FLAGS_RAWDATA: { // pushdata
            interpreter_read(it, sizeof(uint64_t), cache);
            uint64_t sz = *((uint64_t*)cache);

            void *data = malloc(sz); // managed by refcounter

            interpreter_read(it, sz, data);

            value_setRefCounted(rt, &stack->data[stackLen], data);

            break;
          }
        }

        // TODO: make stack size dynamic?
        assert(((stackLen + 1) * sizeof(value_t) < STACK_SIZE_BYTES) && "stack overflow");

        ++*stack->lenVal;
        //++stack->len;

        break;
      }

      case OP_POP: {
        // assert(stack.len != 0);

        storage_t *s = &rt->dt->storage[AT_LOCAL];

        interpreter_read(it, sizeof(uint16_t), cache);
        uint16_t sz = *((uint16_t*)cache);

        while (sz--) { // required to call free() on malloc'd objects
          value_destroy(rt, &s->data[--*s->lenVal]);
        }

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

      case OP_CALL: {
        obj_loc_t o;
        loc_28_t loc;
        archtype_t at;

        interpreter_read(it, sizeof(o), &o);
        obj_loc_parse(o, &loc, &at);

        value_t result = value_invoke(rt, datatable_getValue(rt->dt, loc, at));

        rt->dt->storage[AT_REG].data[0] = result;

        // @NOTE: reason we are NOT doing value_copyValue() here, is because we want the register value to inherit
        // all responsibilities of `result` here ... including refcounts, free() obligations...

        break;
      }

      case OP_PRINT: {
        obj_loc_t o;
        loc_28_t loc;
        archtype_t at;

        interpreter_read(it, sizeof(o), &o);
        obj_loc_parse(o, &loc, &at);

        value_t *v = datatable_getValue(rt->dt, loc, at);

        switch (value_getType(v)) {
          case TYPE_NONE:
            printf("none");
            break;
          case TYPE_INT:
            printf("%d", value_getInt(v));
            break;
          case TYPE_UINT:
            printf("%zu", value_getUint(v));
            break;
          case TYPE_DOUBLE:
            printf("%0.f", value_getDouble(v));
            break;
          case TYPE_BOOLEAN:
            printf(value_getBoolean(v) ? "true" : "false");
            break;
          default:
            printf("%p", value_getRawPointer(v));
            break;
        }

        break;
      }

      // ...

      case OP_JIT: {
        assert(false && "JIT unimplemented");
#if 0
        // @jit_begin("sum", memoized=true, args=1)
        // ... some instructions
        // @jit_end ; internally -- tags `sum` as $d6

        // -> OP_JIT JIT_FLAG_BEGIN | JIT_FLAG_MEMOIZE ; memoize for when fn is called ?? idk
        // ->   1 ; args to hash for memoize
        // ->   $d6 ; location of native fn to store at
        // -> ... some instructions
        // -> OP_JIT JIT_FLAG_END

        // ; raw, unoptimized code

        // -> OP_CALL #{_System_arrayCreate} ; construct an array -- now stored in $r0
        // -> OP_PUSH $r0 ; push new array to stack -- stored as local variable

        // -> OP_PUSH $s[-1] ; push last stack item to top, duplicating
        // -> OP_LOAD | CONST_FLAG_U64 $r0 0
        // -> OP_PUSH $r0 ; push 0 to stack
        // -> OP_LOAD | CONST_FLAG_F64 $r0 3.5
        // -> OP_PUSH $r0 ; push 3.5 to stack
        // -> OP_CALL #{_System_arraySetIndex} ; set array[0] to 3.5
        // -> OP_POP 3

        // -> OP_PUSH $s[-1] ; push last stack item to top, duplicating
        // -> OP_LOAD | CONST_FLAG_U64 $r0 0
        // -> OP_PUSH $r0 ; push 0 to stack
        // -> OP_LOAD | CONST_FLAG_F64 $r0 4.6
        // -> OP_PUSH $r0 ; push 4.6 to stack
        // -> OP_CALL #{_System_arraySetIndex} ; set array[0] to 4.6
        // -> OP_POP 3

        // -> OP_CALL #{sum} ; call native fn

        // =====
        // ; optimized, first pass

        // -> OP_CALL #{_System_arrayCreate} ; construct an array -- now stored in $r0
        // -> OP_PUSH $r0 ; push new array to stack -- stored as local variable

        // -> OP_PUSH $s[-1] ; push last stack item to top, duplicating
        // -> OP_PUSH | CONST_FLAG_U64 0
        // -> OP_PUSH | CONST_FLAG_F64 3.5 ; push 3.5 to stack
        // -> OP_CALL #{_System_arraySetIndex} ; set array[0] to 3.5
        // -> OP_POP 3

        // -> OP_PUSH $s[-1] ; push last stack item to top, duplicating
        // -> OP_PUSH | CONST_FLAG_U64 0 ; push 0 to stack
        // -> OP_PUSH | CONST_FLAG_F64 4.6 ; push 4.6 to stack
        // -> OP_CALL #{_System_arraySetIndex} ; set array[0] to 4.6
        // -> OP_POP 3

        // -> OP_CALL #{sum} ; call native fn


        // built a native_function_t jitFunction_<addr>
        // @todo: function memoization? maybe have an argument for X number of stack parameters
        // to take a hash of and lookup a memoized function based on this?

        native_function_t jitFunction;

        if (flags & JIT_FLAG_BEGIN) { // begin
          char *buf; // C source code buffer?
          uint8_t jitOp = OP_JIT;
          uint8_t jitFlags = flags;


          // set hash to be hash of (it->pc)
          // as well as hashcode of X number of stack items
          // this could be useful for memoizing function arguments
          //char hash[64];

          // while (!interpreter_atEnd(it)) {
          //   interpreter_read(it, &jitOp, &jitFlags);

          //   if (flags & JIT_FLAG_MEMOIZE) {
          //     jit_buildMemoized(rt->jit, jitOp, jitFlags, &buf);
          //   } else {
          //     // default -- does not expand CMP and conditionals
          //     jit_build(rt->jit, jitOp, jitFlags, &buf);
          //   }
          // }

          // interpreter_runJit(it);

          // first pass, run C compiler, load obj file, lookup function `jitFunction_<hash>`


          // jitmap_set(rt->jit->map, )
        }

        // move value_t to static data that holds jitFunction

#endif

        break;
      }

      case OP_HALT:
        exit(0);
        break;
    }
  }
}
