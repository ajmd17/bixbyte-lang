#include <bcparse/emit/emit.hpp>
#include <bcparse/emit/formatter.hpp>

namespace bcparse {
  Op_Sub::Op_Sub(const ObjLoc &left,
    const ObjLoc &right,
    Op_Cmp::Flags flags)
    : m_left(left),
      m_right(right),
      m_flags(flags) {
  }

  void Op_Sub::accept(BytecodeStream *bs) {
    Buildable::accept(bs);

    bs->acceptInstruction(0x9, (uint8_t)m_flags);
    bs->acceptObjLoc(m_left);
    bs->acceptObjLoc(m_right);
  }

  void Op_Sub::debugPrint(BytecodeStream *bs, Formatter *f) {
    Buildable::debugPrint(bs, f);

    f->append(std::string("Op_Sub(")
      + m_left.toString()
      + ", "
      + m_right.toString()
      + ")");
  }
}
