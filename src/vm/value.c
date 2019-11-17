#include <vm/value.h>
#include <vm/runtime.h>
#include <vm/obj_loc.h>

void value_destroy(runtime_t *rt, value_t *value) {
  /*if (value->metadata & (TYPE_POINTER | (FLAG_OBJECT << 8))) {
    object_destroy((object_t*)value->data.ptr);
  } else*/
  if ((value->metadata & (TYPE_POINTER | (FLAG_REFCOUNTED << 8))) == (TYPE_POINTER | (FLAG_REFCOUNTED << 8))) {
    runtime_release(rt, value->data.rc);
  } else if ((value->metadata & (TYPE_POINTER | (FLAG_MALLOC << 8))) == (TYPE_POINTER | (FLAG_MALLOC << 8))) {
    free(value->data.raw);
  }
}

void value_copyValue(runtime_t *rt, value_t *v, value_t *other) {
  value_destroy(rt, v);

  if ((other->metadata & (TYPE_POINTER | (FLAG_REFCOUNTED << 8))) == (TYPE_POINTER | (FLAG_REFCOUNTED << 8))) {
    v->data.rc = runtime_claim(rt, (rcmap_key_t)other->data.rc);
  } else {
    v->data = other->data;
  }

  v->metadata = other->metadata;
}

void value_setInt(runtime_t *rt, value_t *v, int64_t i64) {
  value_destroy(rt, v);
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

void value_setUint(runtime_t *rt, value_t *v, uint64_t u64) {
  value_destroy(rt, v);
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

void value_setDouble(runtime_t *rt, value_t *v, double dbl) {
  value_destroy(rt, v);
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

void value_setBoolean(runtime_t *rt, value_t *v, bool b) {
  value_destroy(rt, v);
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

value_t value_createObject(heap_t *heap) {
  value_t v;
  v.data.hv = heap_alloc(heap);
  v.metadata = TYPE_POINTER | (FLAG_OBJECT << 8);
  return v;
}

void *value_getRawPointer(value_t *value) {
  return value->data.raw;
}

void value_setRawPointer(runtime_t *rt, value_t *v, void *raw, VALUE_FLAGS flags) {
  value_destroy(rt, v);
  v->data.raw = raw;
  v->metadata = TYPE_POINTER | (flags << 8);
}

value_t value_fromRawPointer(void *raw, VALUE_FLAGS flags) {
  value_t v;
  v.data.raw = raw;
  v.metadata = TYPE_POINTER | (flags << 8);
  return v;
}

void value_setRefCounted(runtime_t *rt, value_t *v, void *ptr) {
  value_destroy(rt, v);
  v->data.rc = runtime_claim(rt, (rcmap_key_t)ptr);
  v->metadata = TYPE_POINTER | (FLAG_REFCOUNTED << 8);
}

value_t value_fromFunction(native_function_t fn) {
  value_t v;
  v.data.fn = fn;
  v.metadata = TYPE_FUNCTION;
  return v;
}

void value_setFunction(runtime_t *rt, value_t *v, native_function_t fn) {
  value_destroy(rt, v);
  v->data.fn = fn;
  v->metadata = TYPE_FUNCTION;
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
  VALUE_TYPE type = value_getType(value);
  VALUE_FLAGS flags = value_getFlags(value);

  if (type == TYPE_POINTER) {
    if (flags & FLAG_REFCOUNTED) {
      return (uintptr_t)value->data.rc;
    } else if (flags & FLAG_OBJECT) {
      // assert value->data.hv != NULL
      return (uintptr_t)value->data.hv->ptr;
    }

    return (uintptr_t)value->data.raw;
  } else if (type == TYPE_FUNCTION) {
    return (uintptr_t)value->data.fn;
  }

  return (uintptr_t)(&value->data);

  //int isptr = !!(value->metadata & TYPE_POINTER);
  //return (-isptr & (uintptr_t)value->data.ptr) | ((isptr - 1) & (uintptr_t)(&value->data));
}

value_t value_invoke(runtime_t *r, value_t *value) {
  args_t args;
  args._stack = &r->dt->storage[AT_LOCAL];

  return value->data.fn(r, &args);
}
