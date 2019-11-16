#include <bcparse/emit/emit.hpp>
#include <bcparse/emit/formatter.hpp>

namespace bcparse {
  Op_PushConst::Op_PushConst(const Value &arg)
    : m_arg(arg) {
  }

  void Op_PushConst::accept(BytecodeStream *bs) {
    Buildable::accept(bs);

    bs->acceptInstruction(0x6, (uint8_t)m_arg.getValueType());

    if (m_arg.getValueType() == Value::ValueType::ValueTypeRawData) {
      bs->acceptBytes((uint64_t)m_arg.getRawBytes().size());
    }

    for (uint8_t u8 : m_arg.getRawBytes()) {
      bs->acceptBytes(u8);
    }
  }

  void Op_PushConst::debugPrint(BytecodeStream *bs, Formatter *f) {
    Buildable::debugPrint(bs, f);

    f->append(std::string("Op_PushConst(")
      + m_arg.toString()
      + ")");
  }
}
