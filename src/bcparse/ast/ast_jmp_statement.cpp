#include <bcparse/ast/ast_jmp_statement.hpp>
#include <bcparse/ast/ast_identifier.hpp>
#include <bcparse/ast/ast_label.hpp>

#include <bcparse/ast_visitor.hpp>
#include <bcparse/compilation_unit.hpp>
#include <bcparse/emit/emit.hpp>

#include <common/my_assert.hpp>

namespace bcparse {
  AstJmpStatement::AstJmpStatement(Pointer<AstExpression> arg,
    JumpMode jumpMode,
    const SourceLocation &location)
    : AstStatement(location),
      m_arg(arg),
      m_jumpMode(jumpMode),
      m_pointee(nullptr) {
  }

  void AstJmpStatement::visit(AstVisitor *visitor, Module *mod) {
    ASSERT(m_arg != nullptr);

    m_arg->visit(visitor, mod);

    if (auto asIdent = std::dynamic_pointer_cast<AstIdentifier>(m_arg)) {
      if (asIdent->getValue() != nullptr) {
        if (auto asLabel = std::dynamic_pointer_cast<AstLabel>(asIdent->getValue())) {
          m_pointee = asLabel;
        } else {
          visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
            LEVEL_ERROR,
            Msg_custom_error,
            m_arg->getLocation(),
            "Expression is not a label"
          ));
        }
      }
    } else {
      // TODO: dynamic jumps
      visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
        LEVEL_ERROR,
        Msg_custom_error,
        m_arg->getLocation(),
        "<<jmp>> argument must be a label"
      ));
    }
  }

  void AstJmpStatement::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
    ASSERT(m_pointee != nullptr);

    m_pointee->build(visitor, mod, out);

    out->append(std::unique_ptr<Op_Jmp>(new Op_Jmp(
      m_pointee->getObjLoc(),
      static_cast<Op_Jmp::Flags>(m_jumpMode)
    )));
  }

  void AstJmpStatement::optimize(AstVisitor *visitor, Module *mod) {
    // possible inlining of quick jumps?
  }

  Pointer<AstStatement> AstJmpStatement::clone() const {
    return CloneImpl();
  }
}
