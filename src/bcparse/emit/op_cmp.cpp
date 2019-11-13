#include <bcparse/emit/emit.hpp>
#include <bcparse/emit/formatter.hpp>

namespace bcparse {
  Op_Cmp::Op_Cmp(const ObjLoc &left, const ObjLoc &right)
    : m_left(left),
      m_right(right) {
  }

  void Op_Cmp::accept(BytecodeStream *bs) {
    Buildable::accept(bs);

    // TODO: flags!!

    bs->acceptInstruction(0x4);
    bs->acceptObjLoc(m_left);
    bs->acceptObjLoc(m_right);
  }

  void Op_Cmp::debugPrint(BytecodeStream *bs, Formatter *f) {
    Buildable::debugPrint(bs, f);

    f->append(std::string("Op_Cmp(")
      + m_left.toString()
      + ", "
      + m_right.toString()
      + ")");
  }
}
