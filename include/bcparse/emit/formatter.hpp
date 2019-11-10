#pragma once

#include <ostream>
#include <sstream>
#include <string>

namespace bcparse {
  class Formatter {
  public:
    Formatter();

    void increaseIndent();
    void decreaseIndent();

    inline void setLineNo(int lineNo) { m_lineNo = lineNo; }
    inline int getLineNo() const { return m_lineNo; }

    void append(const std::string &str);

    std::string toString() const;

  private:
    int m_lineNo;

    int m_indentation;
    std::stringstream m_buf;
  };
}
