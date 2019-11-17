#include <bcparse/emit/emit.hpp>
#include <bcparse/emit/formatter.hpp>

namespace bcparse {
  Op_Mod::Op_Mod(const ObjLoc &left, const ObjLoc &right)
    : m_left(left),
      m_right(right) {
  }

  void Op_Mod::accept(BytecodeStream *bs) {
    Buildable::accept(bs);

    bs->acceptInstruction(0xC);
    bs->acceptObjLoc(m_left);
    bs->acceptObjLoc(m_right);
  }

  void Op_Mod::debugPrint(BytecodeStream *bs, Formatter *f) {
    Buildable::debugPrint(bs, f);

    f->append(std::string("Op_Mod(")
      + m_left.toString()
      + ", "
      + m_right.toString()
      + ")");
  }
}
