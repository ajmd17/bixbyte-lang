#include <bcparse/emit/emit.hpp>
#include <bcparse/emit/formatter.hpp>

namespace bcparse {
  Op_Shl::Op_Shl(const ObjLoc &left, const ObjLoc &right)
    : m_left(left),
      m_right(right) {
  }

  void Op_Shl::accept(BytecodeStream *bs) {
    Buildable::accept(bs);

    bs->acceptInstruction(0x10);
    bs->acceptObjLoc(m_left);
    bs->acceptObjLoc(m_right);
  }

  void Op_Shl::debugPrint(BytecodeStream *bs, Formatter *f) {
    Buildable::debugPrint(bs, f);

    f->append(std::string("Op_Shl(")
      + m_left.toString()
      + ", "
      + m_right.toString()
      + ")");
  }
}
