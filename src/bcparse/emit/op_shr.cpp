#include <bcparse/emit/emit.hpp>
#include <bcparse/emit/formatter.hpp>

namespace bcparse {
  Op_Shr::Op_Shr(const ObjLoc &left, const ObjLoc &right)
    : m_left(left),
      m_right(right) {
  }

  void Op_Shr::accept(BytecodeStream *bs) {
    Buildable::accept(bs);

    bs->acceptInstruction(0x11);
    bs->acceptObjLoc(m_left);
    bs->acceptObjLoc(m_right);
  }

  void Op_Shr::debugPrint(BytecodeStream *bs, Formatter *f) {
    Buildable::debugPrint(bs, f);

    f->append(std::string("Op_Shr(")
      + m_left.toString()
      + ", "
      + m_right.toString()
      + ")");
  }
}
