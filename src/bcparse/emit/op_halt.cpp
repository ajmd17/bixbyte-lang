#include <bcparse/emit/emit.hpp>
#include <bcparse/emit/formatter.hpp>

namespace bcparse {
  Op_Halt::Op_Halt() {
  }

  void Op_Halt::accept(BytecodeStream *bc) {
    bc->acceptInstruction(0x1F);
  }

  void Op_Halt::debugPrint(Formatter *f) {
    f->append("Op_Halt()");
  }
}
