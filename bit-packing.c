#include <stdint>

enum ARCHETYPE {
  AT_DATA = 0x1,
  AT_LOCAL = 0x2,
  AT_REG = 0x3,

  AT_REL = 0x8,
  AT_ABS = 0xC
};

typedef int32_t obj_loc_t;
typedef uint8_t archtype_t;
typedef int32_t loc_28_t;

obj_loc_t pie_make_obj_loc(loc_28_t loc, archtype_t at) {
  obj_loc_t payload = loc;
  payload <<= 4;
  payload |= at;
  return payload;
}

void pie_parse_obj_loc(obj_loc_t payload, loc_28_t *loc, archtype_t *at) {
  *loc = payload >> 4;
  *at = payload & 0xF;
}

// ===============
int main() {
	obj_loc_t o = pie_make_obj_loc(123/* 28-bit int */, AT_DATA | AT_ABS);
	
	loc_28_t loc;
	archtype_t at;
	pie_parse_obj_loc(o, &loc, &at);

	return 0;
}
