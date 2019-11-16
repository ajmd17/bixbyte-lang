#include <bcparse/emit/value.hpp>

namespace bcparse {
  const Value Value::none() {
    Value v;
    v.m_valueType = ValueType::ValueTypeNone;
    v.m_rawBytes.clear();
    return v;
  }
}
