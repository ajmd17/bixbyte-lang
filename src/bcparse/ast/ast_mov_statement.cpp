#include <bcparse/ast/ast_mov_statement.hpp>
#include <bcparse/ast/ast_variable.hpp>
#include <bcparse/ast/ast_label.hpp>

#include <bcparse/ast_visitor.hpp>
#include <bcparse/compilation_unit.hpp>
#include <bcparse/emit/emit.hpp>

#include <common/my_assert.hpp>

namespace bcparse {
  AstMovStatement::AstMovStatement(Pointer<AstExpression> left,
    Pointer<AstExpression> right,
    const SourceLocation &location)
    : AstStatement(location),
      m_left(left),
      m_right(right) {
  }

  void AstMovStatement::visit(AstVisitor *visitor, Module *mod) {
    ASSERT(m_left != nullptr);
    ASSERT(m_right != nullptr);

    m_left->visit(visitor, mod);
    m_right->visit(visitor, mod);
  }

  void AstMovStatement::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
    ASSERT(m_left != nullptr);
    ASSERT(m_right != nullptr);

    m_left->build(visitor, mod, out);
    m_right->build(visitor, mod, out);

    out->append(std::unique_ptr<Op_Mov>(new Op_Mov(
      m_left->getObjLoc(),
      m_right->getObjLoc()
    )));
  }

  void AstMovStatement::optimize(AstVisitor *visitor, Module *mod) {
    ASSERT(m_left != nullptr);
    ASSERT(m_right != nullptr);

    m_left->optimize(visitor, mod);
    m_right->optimize(visitor, mod);
  }

  Pointer<AstStatement> AstMovStatement::clone() const {
    return CloneImpl();
  }
}
