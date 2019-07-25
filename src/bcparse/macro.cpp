#include <bcparse/macro.hpp>

namespace bcparse {
  Macro::Macro(const std::string &name, const std::string &body)
    : m_name(name),
      m_body(body) {
  }

  Macro::Macro(const Macro &other)
    : m_name(other.m_name),
      m_body(other.m_body) {
  }
}
