#include <bcparse/ast/ast_unset.hpp>

#include <bcparse/ast_visitor.hpp>
#include <bcparse/compilation_unit.hpp>

#include <bcparse/emit/emit.hpp>

#include <sstream>

namespace bcparse {
  AstUnset::AstUnset(const SourceLocation &location)
    : AstExpression(location) {
  }

  void AstUnset::visit(AstVisitor *visitor, Module *mod) {
    visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
      LEVEL_ERROR,
      Msg_custom_error,
      m_location,
      "unset var used"
    ));
  }

  void AstUnset::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
  }

  void AstUnset::optimize(AstVisitor *visitor, Module *mod) {
  }

  Pointer<AstStatement> AstUnset::clone() const {
    return CloneImpl();
  }

  std::string AstUnset::toString() const {
    return "unset";
  }
}
