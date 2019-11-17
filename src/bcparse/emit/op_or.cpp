#include <bcparse/emit/emit.hpp>
#include <bcparse/emit/formatter.hpp>

namespace bcparse {
  Op_Or::Op_Or(const ObjLoc &left, const ObjLoc &right)
    : m_left(left),
      m_right(right) {
  }

  void Op_Or::accept(BytecodeStream *bs) {
    Buildable::accept(bs);

    bs->acceptInstruction(0xF);
    bs->acceptObjLoc(m_left);
    bs->acceptObjLoc(m_right);
  }

  void Op_Or::debugPrint(BytecodeStream *bs, Formatter *f) {
    Buildable::debugPrint(bs, f);

    f->append(std::string("Op_Or(")
      + m_left.toString()
      + ", "
      + m_right.toString()
      + ")");
  }
}
