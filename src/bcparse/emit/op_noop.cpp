#include <bcparse/emit/emit.hpp>
#include <bcparse/emit/formatter.hpp>

namespace bcparse {
  Op_NoOp::Op_NoOp() {
  }

  void Op_NoOp::accept(BytecodeStream *bs) {
    Buildable::accept(bs);

    bs->acceptInstruction(0x0);
  }

  void Op_NoOp::debugPrint(BytecodeStream *bs, Formatter *f) {
    Buildable::debugPrint(bs, f);

    f->append("Op_NoOp()");
  }
}
