#include <bcparse/ast/ast_directive.hpp>

namespace bcparse {
  AstDirective::AstDirective(const std::string &name,
    const std::vector<Pointer<AstExpression>> &arguments,
    const std::string &body,
    const SourceLocation &location)
    : AstStatement(location),
      m_name(name),
      m_arguments(arguments),
      m_body(body) { 
  }

  void AstDirective::visit(AstVisitor *visitor, Module *mod) {

  }

  void AstDirective::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {

  }

  void AstDirective::optimize(AstVisitor *visitor, Module *mod) {

  }

  Pointer<AstStatement> AstDirective::clone() const {
    return CloneImpl();
  }
}
