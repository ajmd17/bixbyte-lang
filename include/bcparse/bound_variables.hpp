#pragma once

#include <bcparse/ast/ast_expression.hpp>

#include <bcparse/token.hpp>
#include <bcparse/macro.hpp>

#include <map>
#include <string>
#include <vector>
#include <memory>

template <typename T>
using Pointer = std::shared_ptr<T>;

namespace bcparse {
  class BoundVariables {
  public:
    BoundVariables();
    BoundVariables(const BoundVariables &other);

    void setParent(BoundVariables *parent);
    BoundVariables *getParent();

    Pointer<AstExpression> get(const std::string &name, bool bubbles = true);
    void set(const std::string &name, const Pointer<AstExpression> &value);

    void defineMacro(const std::string &name, const std::vector<Token> &body);
    Macro *lookupMacro(const std::string &name);

    std::map<std::string, Pointer<AstExpression>> &getMap() { return m_map; }

  private:
    BoundVariables *m_parent;
    std::map<std::string, Pointer<AstExpression>> m_map;
    std::map<std::string, std::shared_ptr<Macro>> m_macros;
  };
}
