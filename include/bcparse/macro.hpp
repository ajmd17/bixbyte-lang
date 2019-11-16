#pragma once

#include <bcparse/token.hpp>

#include <string>
#include <vector>

namespace bcparse {
  class Macro {
  public:
    Macro(const std::string &name, const std::vector<Token> &body);
    Macro(const Macro &other);

    inline const std::string &getName() const { return m_name; }
    inline const std::vector<Token> &getBody() const { return m_body; }

  private:
    std::string m_name;
    std::vector<Token> m_body;
  };
}
