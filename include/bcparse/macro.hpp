#pragma once

#include <string>

namespace bcparse {
  class Macro {
  public:
    Macro(const std::string &name, const std::string &body);
    Macro(const Macro &other);

    inline const std::string &getName() const { return m_name; }
    inline const std::string &getBody() const { return m_body; }
  
  private:
    std::string m_name;
    std::string m_body;
  };
}
