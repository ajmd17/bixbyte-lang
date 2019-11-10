#pragma once

#include <vm/object.h>
#include <vm/value.h>

typedef struct {
  object_t *base;
  value_t argument;
} exception_t;

exception_t exception_fromValue(value_t *argument);
