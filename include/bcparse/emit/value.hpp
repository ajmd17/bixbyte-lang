#pragma once

#include <vector>
#include <cstring>

namespace bcparse {
  class Value {
  public:
    Value(std::nullptr_t = nullptr) {
      m_valueType = ValueType::ValueTypeNull;
    }

    Value(const Value &other)
      : m_valueType(other.m_valueType),
        m_rawBytes(other.m_rawBytes) {
    }

    explicit Value(int64_t i64) {
      m_valueType = ValueType::ValueTypeI64;
      m_rawBytes.resize(sizeof(i64));
      std::memcpy(m_rawBytes.data(), &i64, sizeof(i64));
    }

    explicit Value(uint64_t u64) {
      m_valueType = ValueType::ValueTypeU64;
      m_rawBytes.resize(sizeof(u64));
      std::memcpy(m_rawBytes.data(), &u64, sizeof(u64));
    }

    explicit Value(double f64) {
      m_valueType = ValueType::ValueTypeF64;
      m_rawBytes.resize(sizeof(f64));
      std::memcpy(m_rawBytes.data(), &f64, sizeof(f64));
    }

    explicit Value(bool b) {
      uint8_t u8 = (uint8_t)b;
      m_valueType = ValueType::ValueTypeBoolean;
      m_rawBytes.resize(sizeof(u8));
      std::memcpy(m_rawBytes.data(), &u8, sizeof(u8));
    }

    explicit Value(const std::vector<uint8_t> &data) {
      m_valueType = ValueType::ValueTypeRawData;
      m_rawBytes = data;
    }

    enum class ValueType {
      ValueTypeNull = 0x0,
      ValueTypeI64 = 0x1,
      ValueTypeU64 = 0x2,
      ValueTypeF64 = 0x3,
      ValueTypeBoolean = 0x4,
      // empty space
      ValueTypeRawData = 0x7
    };

    inline ValueType getValueType() const { return m_valueType; }
    inline const std::vector<uint8_t> &getRawBytes() const { return m_rawBytes; }

  private:
    ValueType m_valueType;
    std::vector<uint8_t> m_rawBytes;
  };
}
