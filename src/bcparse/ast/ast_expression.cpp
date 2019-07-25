#include <bcparse/ast/ast_expression.hpp>

namespace bcparse {
  AstExpression::AstExpression(const SourceLocation &location)
    : AstStatement(location) {
  }
}
