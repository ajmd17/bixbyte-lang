#include <bcparse/emit/formatter.hpp>

namespace bcparse {
  Formatter::Formatter()
    : m_indentation(0),
      m_lineNo(-1) {
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
    if (m_lineNo != -1) {
      const size_t fillLength = 6;
      char border[fillLength] = { '\0' };

      snprintf(border, fillLength, "%d", m_lineNo);

      int len = strlen(border);

      for (int i = len; i < fillLength - 1; i++) {
        border[i] = ' ';
      }

      border[fillLength - 1] = '\0';

      m_buf << border;
    }

    for (int i = 0; i < m_indentation; i++) {
      m_buf << "  ";
    }

    m_buf << str << "\n";
  }

  std::string Formatter::toString() const {
    return m_buf.str();
  }
}
