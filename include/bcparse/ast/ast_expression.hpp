#pragma once

#include <bcparse/ast/ast_statement.hpp>

namespace bcparse {
  class AstExpression : public AstStatement {
  public:
    AstExpression(const SourceLocation &location);
  };
}
