#include <bcparse/emit/emit.hpp>
#include <bcparse/emit/formatter.hpp>

namespace bcparse {
  Op_Mul::Op_Mul(const ObjLoc &left, const ObjLoc &right)
    : m_left(left),
      m_right(right) {
  }

  void Op_Mul::accept(BytecodeStream *bs) {
    Buildable::accept(bs);

    bs->acceptInstruction(0xA);
    bs->acceptObjLoc(m_left);
    bs->acceptObjLoc(m_right);
  }

  void Op_Mul::debugPrint(BytecodeStream *bs, Formatter *f) {
    Buildable::debugPrint(bs, f);

    f->append(std::string("Op_Mul(")
      + m_left.toString()
      + ", "
      + m_right.toString()
      + ")");
  }
}
