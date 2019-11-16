#include <bcparse/ast/ast_expression.hpp>

namespace bcparse {
  AstExpression::AstExpression(const SourceLocation &location)
    : AstStatement(location) {
  }

  AstExpression *AstExpression::getValueOf() {
    return this;
  }

  AstExpression *AstExpression::getDeepValueOf() {
    return getValueOf();
  }

  Value AstExpression::getRuntimeValue() const {
    return Value::none();
  }

  std::string AstExpression::toString() const {
    return typeid(*this).name();
  }
}
