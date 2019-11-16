#include <bcparse/emit/emit.hpp>
#include <bcparse/emit/formatter.hpp>

namespace bcparse {
  Op_Push::Op_Push(const ObjLoc &arg)
    : m_arg(arg) {
  }

  void Op_Push::accept(BytecodeStream *bs) {
    Buildable::accept(bs);

    bs->acceptInstruction(0x6, (uint8_t)Value::ValueType::ValueTypeNone);
    bs->acceptObjLoc(m_arg);
  }

  void Op_Push::debugPrint(BytecodeStream *bs, Formatter *f) {
    Buildable::debugPrint(bs, f);

    f->append(std::string("Op_Push(")
      + m_arg.toString()
      + ")");
  }
}
