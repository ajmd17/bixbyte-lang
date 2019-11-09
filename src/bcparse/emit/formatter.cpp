#include <bcparse/emit/formatter.hpp>

namespace bcparse {
  Formatter::Formatter()
    : m_indentation(0) {
  }

  void Formatter::increaseIndent() {
    ++m_indentation;
  }

  void Formatter::decreaseIndent() {
    if (m_indentation > 0) {
      --m_indentation;
    }
  }

  void Formatter::append(const std::string &str) {
    for (int i = 0; i < m_indentation; i++) {
      m_buf << "  ";
    }

    m_buf << str << "\n";
  }

  std::string Formatter::toString() const {
    return m_buf.str();
  }
}
