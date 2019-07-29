#include <bcparse/emit/emit.hpp>

namespace bcparse {
  Op_Load::Op_Load(const ObjLoc &objLoc, const Value &value)
    : m_objLoc(objLoc),
      m_value(value) {
  }

  void Op_Load::accept(BytecodeStream *bs) {
    bs->acceptInstruction(0x1, (uint8_t)m_value.getValueType());
    bs->acceptObjLoc(m_objLoc);

    if (m_value.getValueType() == Value::ValueType::ValueTypeRawData) {
      bs->acceptBytes((uint64_t)m_value.getRawBytes().size());
    }

    for (uint8_t u8 : m_value.getRawBytes()) {
      bs->acceptBytes(u8);
    }
  }
}
