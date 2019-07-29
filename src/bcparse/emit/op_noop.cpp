#include <bcparse/emit/emit.hpp>

namespace bcparse {
  Op_NoOp::Op_NoOp() {
  }

  void Op_NoOp::accept(BytecodeStream *bc) {
    bc->acceptInstruction(0x0);
  }
}
