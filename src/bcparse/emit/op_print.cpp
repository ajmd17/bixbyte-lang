#include <bcparse/emit/emit.hpp>
#include <bcparse/emit/formatter.hpp>

namespace bcparse {
  Op_Print::Op_Print(const ObjLoc &objLoc)
    : m_objLoc(objLoc) {
  }

  void Op_Print::accept(BytecodeStream *bs) {
    Buildable::accept(bs);

    bs->acceptInstruction(0x15);
    bs->acceptObjLoc(m_objLoc);
  }

  void Op_Print::debugPrint(BytecodeStream *bs, Formatter *f) {
    Buildable::debugPrint(bs, f);

    f->append(std::string("Op_Print(")
      + m_objLoc.toString()
      + ")");
  }
}
