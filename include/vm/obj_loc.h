#pragma once

#include <stdint.h>

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

obj_loc_t obj_loc_make(loc_28_t loc, archtype_t at);
void obj_loc_parse(obj_loc_t payload, loc_28_t *loc, archtype_t *at);
