#pragma once

namespace bcparse {
  class RelativeStackOffset {
  public:
    RelativeStackOffset() : m_value(0) {}
    RelativeStackOffset(const RelativeStackOffset &other) : m_value(other.m_value) {}

    inline int current() const { return m_value; }
    inline int inc() { return ++m_value; }
    inline int dec() { return --m_value; }

  private:
    int m_value;
  };
}
