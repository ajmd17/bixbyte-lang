#include <bcparse/emit/emit.hpp>

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
}
