#include <vm/except.h>

exception_t exception_fromValue(value_t *argument) {
  exception_t e;
  e.base = NULL; // @TODO SimpleException
  e.argument = *argument;
  return e;
}
