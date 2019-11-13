#include <bcparse/ast/ast_statement.hpp>

namespace bcparse {
  AstStatement::AstStatement(const SourceLocation &location)
    : m_location(location) {
  }

  bool AstStatement::isHoisted() const {
    return false;
  }
}
