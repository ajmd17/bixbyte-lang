#include <bcparse/ast/ast_identifier.hpp>

#include <bcparse/ast_visitor.hpp>
#include <bcparse/compilation_unit.hpp>

#include <bcparse/emit/emit.hpp>

namespace bcparse {
  AstIdentifier::AstIdentifier(const std::string &name,
    const SourceLocation &location)
    : AstExpression(location),
      m_name(name),
      m_value(nullptr) {
  }

  void AstIdentifier::visit(AstVisitor *visitor, Module *mod) {
    if (auto ptr = visitor->getCompilationUnit()->getBoundGlobals().get(m_name)) {
      m_value = ptr;
      m_value->visit(visitor, mod);

      return;
    }

    visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
      LEVEL_ERROR,
      Msg_undeclared_identifier,
      m_location,
      m_name
    ));
  }

  void AstIdentifier::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
    if (m_value != nullptr) {
      m_value->build(visitor, mod, out);

      m_objLoc = m_value->getObjLoc();
    }
  }

  void AstIdentifier::optimize(AstVisitor *visitor, Module *mod) {
    // TODO: inlining?
  }

  Pointer<AstStatement> AstIdentifier::clone() const {
    return CloneImpl();
  }
}
