#include <bcparse/emit/emit.hpp>
#include <bcparse/emit/formatter.hpp>

namespace bcparse {
  Op_Halt::Op_Halt() {
  }

  void Op_Halt::accept(BytecodeStream *bs) {
    Buildable::accept(bs);

    bs->acceptInstruction(0x1F);
  }

  void Op_Halt::debugPrint(BytecodeStream *bs, Formatter *f) {
    Buildable::debugPrint(bs, f);

    f->append("Op_Halt()");
  }
}
