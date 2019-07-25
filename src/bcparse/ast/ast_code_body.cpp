#include <bcparse/ast/ast_code_body.hpp>

namespace bcparse {
  AstCodeBody::AstCodeBody(const std::string &value, const SourceLocation &location)
    : AstStringLiteral(value, location) {
  }

  void AstCodeBody::visit(AstVisitor *visitor, Module *mod) {
    // @TODO run lexer + parser internally
  }
}
