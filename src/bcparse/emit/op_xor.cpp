#include <bcparse/emit/emit.hpp>
#include <bcparse/emit/formatter.hpp>

namespace bcparse {
  Op_Xor::Op_Xor(const ObjLoc &left, const ObjLoc &right)
    : m_left(left),
      m_right(right) {
  }

  void Op_Xor::accept(BytecodeStream *bs) {
    Buildable::accept(bs);

    bs->acceptInstruction(0xD);
    bs->acceptObjLoc(m_left);
    bs->acceptObjLoc(m_right);
  }

  void Op_Xor::debugPrint(BytecodeStream *bs, Formatter *f) {
    Buildable::debugPrint(bs, f);

    f->append(std::string("Op_Xor(")
      + m_left.toString()
      + ", "
      + m_right.toString()
      + ")");
  }
}
