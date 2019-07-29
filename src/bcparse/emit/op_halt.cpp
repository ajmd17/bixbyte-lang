#include <bcparse/emit/emit.hpp>

namespace bcparse {
  Op_Halt::Op_Halt() {
  }

  void Op_Halt::accept(BytecodeStream *bc) {
    bc->acceptInstruction(0x1F);
  }
}
