#include <bcparse/ast/ast_jmp_statement.hpp>
#include <bcparse/ast/ast_variable.hpp>
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

    // TODO: if arg is an AstSymbol, we look for a Label with that name
  }

  void AstJmpStatement::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
    ASSERT(m_arg != nullptr);

    m_arg->build(visitor, mod, out);

    // TODO: dynamic jumping will require an OP_LOAD

    out->append(std::unique_ptr<Op_Jmp>(new Op_Jmp(
      m_arg->getObjLoc(),
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
