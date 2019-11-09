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

    void append(const std::string &str);

    std::string toString() const;

  private:
    int m_indentation;
    std::stringstream m_buf;
  };
}
