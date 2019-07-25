#pragma once

#include <map>
#include <string>
#include <memory>

#include <bcparse/ast/ast_expression.hpp>

template <typename T>
using Pointer = std::shared_ptr<T>;

namespace bcparse {
  class BoundVariables {
  public:
    BoundVariables();
    BoundVariables(const BoundVariables &other);

    void setParent(BoundVariables *parent);
    BoundVariables *getParent();

    Pointer<AstExpression> get(const std::string &name);
    void set(const std::string &name, const Pointer<AstExpression> &value);

  private:
    BoundVariables *m_parent;
    std::map<std::string, Pointer<AstExpression>> m_map;
  };
}
