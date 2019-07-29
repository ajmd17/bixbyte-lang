#pragma once

namespace bcparse {
  class RegisterUsage {
  public:
    RegisterUsage() : m_value(0) {}
    RegisterUsage(const RegisterUsage &other) : m_value(other.m_value) {}

    inline int current() const { return m_value; }
    inline int inc() { return ++m_value; }
    inline int dec() { return --m_value; }

  private:
    int m_value;
  };
}
