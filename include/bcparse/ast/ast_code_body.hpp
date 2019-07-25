#pragma once

#include <bcparse/ast/ast_string_literal.hpp>

namespace bcparse {
  class AstCodeBody : public AstStringLiteral {
  public:
    AstCodeBody(const std::string &value, const SourceLocation &location);
    virtual ~AstCodeBody() = default;

    virtual void visit(AstVisitor *visitor, Module *mod) override;
  };
}
