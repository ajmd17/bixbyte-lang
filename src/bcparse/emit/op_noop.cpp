#include <bcparse/emit/emit.hpp>
#include <bcparse/emit/formatter.hpp>

namespace bcparse {
  Op_NoOp::Op_NoOp() {
  }

  void Op_NoOp::accept(BytecodeStream *bc) {
    bc->acceptInstruction(0x0);
  }

  void Op_NoOp::debugPrint(Formatter *f) {
    f->append("Op_NoOp()");
  }
}
