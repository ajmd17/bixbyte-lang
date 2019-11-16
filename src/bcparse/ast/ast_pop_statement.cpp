#include <bcparse/ast/ast_pop_statement.hpp>
#include <bcparse/ast/ast_data_location.hpp>

#include <bcparse/ast_visitor.hpp>
#include <bcparse/compilation_unit.hpp>
#include <bcparse/emit/emit.hpp>

#include <common/my_assert.hpp>

namespace bcparse {
  AstPopStatement::AstPopStatement(size_t amt,
    const SourceLocation &location)
    : AstStatement(location),
      m_amt(amt) {
  }

  void AstPopStatement::visit(AstVisitor *visitor, Module *mod) {
    // make sure m_amt fits into a uint16_t
  }

  void AstPopStatement::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
    out->append(std::unique_ptr<Op_Pop>(new Op_Pop(
      m_amt
    )));
  }

  void AstPopStatement::optimize(AstVisitor *visitor, Module *mod) {
  }

  Pointer<AstStatement> AstPopStatement::clone() const {
    return CloneImpl();
  }
}
