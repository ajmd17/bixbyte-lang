#include <bcparse/emit/emit.hpp>
#include <bcparse/emit/formatter.hpp>

namespace bcparse {
  Op_Mov::Op_Mov(const ObjLoc &left, const ObjLoc &right)
    : m_left(left),
      m_right(right) {
  }

  void Op_Mov::accept(BytecodeStream *bs) {
    bs->acceptInstruction(0x2);
    bs->acceptObjLoc(m_left);
    bs->acceptObjLoc(m_right);
  }

  void Op_Mov::debugPrint(Formatter *f) {
    f->append(std::string("Op_Mov(")
      + m_left.toString()
      + ", "
      + m_right.toString()
      + ")");
  }
}
