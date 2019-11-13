#include <bcparse/ast/ast_cmp_statement.hpp>
#include <bcparse/ast/ast_identifier.hpp>
#include <bcparse/ast/ast_label.hpp>

#include <bcparse/ast_visitor.hpp>
#include <bcparse/compilation_unit.hpp>
#include <bcparse/emit/emit.hpp>

#include <common/my_assert.hpp>

namespace bcparse {
  AstCmpStatement::AstCmpStatement(Pointer<AstExpression> left,
    Pointer<AstExpression> right,
    const SourceLocation &location)
    : AstStatement(location),
      m_left(left),
      m_right(right) {
  }

  void AstCmpStatement::visit(AstVisitor *visitor, Module *mod) {
    ASSERT(m_left != nullptr);
    ASSERT(m_right != nullptr);

    m_left->visit(visitor, mod);
    m_right->visit(visitor, mod);

  }

  void AstCmpStatement::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
    ASSERT(m_left != nullptr);
    ASSERT(m_right != nullptr);

    m_left->build(visitor, mod, out);
    visitor->getCompilationUnit()->getRegisterUsage().inc();

    m_right->build(visitor, mod, out);
    visitor->getCompilationUnit()->getRegisterUsage().dec();

    out->append(std::unique_ptr<Op_Cmp>(new Op_Cmp(
      m_left->getObjLoc(),
      m_right->getObjLoc()
    )));
  }

  void AstCmpStatement::optimize(AstVisitor *visitor, Module *mod) {
    // constant folding
  }

  Pointer<AstStatement> AstCmpStatement::clone() const {
    return CloneImpl();
  }
}
