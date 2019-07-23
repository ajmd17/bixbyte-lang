#include <bcparse/compiler_error.hpp>

namespace bcparse {
  CompilerError::CompilerError(const CompilerError &other)
    : m_level(other.m_level),
      m_msg(other.m_msg),
      m_location(other.m_location),
      m_text(other.m_text) {
  }

  bool CompilerError::operator<(const CompilerError &other) const {
    if (m_level == other.m_level) {
      return m_location < other.m_location;
    } else {
      return m_level < other.m_level;
    }
  }
}
