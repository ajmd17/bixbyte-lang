#include <bcparse/ast/ast_expression.hpp>

namespace bcparse {
  AstExpression::AstExpression(const SourceLocation &location)
    : AstStatement(location) {
  }

  AstExpression *AstExpression::getValueOf() {
    return this;
  }

  std::string AstExpression::toString() const {
    return "";
  }
}
