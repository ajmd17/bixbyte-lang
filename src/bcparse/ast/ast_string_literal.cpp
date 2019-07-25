#include <bcparse/ast/ast_string_literal.hpp>

namespace bcparse {
  AstStringLiteral::AstStringLiteral(const std::string &value,
    const SourceLocation &location)
    : AstExpression(location),
      m_value(value) {
  }

  void AstStringLiteral::visit(AstVisitor *visitor, Module *mod) {
  }

  void AstStringLiteral::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
  }

  void AstStringLiteral::optimize(AstVisitor *visitor, Module *mod) {
  }

  Pointer<AstStatement> AstStringLiteral::clone() const {
    return CloneImpl();
  }
}
