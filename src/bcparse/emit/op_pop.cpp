#include <bcparse/emit/emit.hpp>
#include <bcparse/emit/formatter.hpp>

#include <sstream>

namespace bcparse {
  Op_Pop::Op_Pop(size_t amt):
    m_amt(amt) {
  }

  void Op_Pop::accept(BytecodeStream *bs) {
    Buildable::accept(bs);

    bs->acceptInstruction(0x7);

    // TODO: assert m_amt can fit in uint16_t
    bs->acceptBytes((uint16_t)m_amt);
  }

  void Op_Pop::debugPrint(BytecodeStream *bs, Formatter *f) {
    Buildable::debugPrint(bs, f);

    std::stringstream ss;
    ss << "Op_Pop(";
    ss << m_amt;
    ss << ")";

    f->append(ss.str());
  }
}
