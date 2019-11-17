#include <bcparse/emit/emit.hpp>
#include <bcparse/emit/formatter.hpp>

namespace bcparse {
  Op_And::Op_And(const ObjLoc &left, const ObjLoc &right)
    : m_left(left),
      m_right(right) {
  }

  void Op_And::accept(BytecodeStream *bs) {
    Buildable::accept(bs);

    bs->acceptInstruction(0xE);
    bs->acceptObjLoc(m_left);
    bs->acceptObjLoc(m_right);
  }

  void Op_And::debugPrint(BytecodeStream *bs, Formatter *f) {
    Buildable::debugPrint(bs, f);

    f->append(std::string("Op_And(")
      + m_left.toString()
      + ", "
      + m_right.toString()
      + ")");
  }
}
