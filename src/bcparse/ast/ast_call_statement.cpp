#include <bcparse/ast/ast_call_statement.hpp>
#include <bcparse/ast/ast_push_statement.hpp>
#include <bcparse/ast/ast_pop_statement.hpp>
#include <bcparse/ast/ast_variable.hpp>
#include <bcparse/ast/ast_label.hpp>

#include <bcparse/ast_visitor.hpp>
#include <bcparse/compilation_unit.hpp>
#include <bcparse/emit/emit.hpp>

#include <common/my_assert.hpp>

namespace bcparse {
  AstCallStatement::AstCallStatement(std::vector<Pointer<AstExpression>> args,
    const SourceLocation &location)
    : AstStatement(location),
      m_args(args) {
  }

  void AstCallStatement::visit(AstVisitor *visitor, Module *mod) {
    for (auto &arg : m_args) {
      ASSERT(arg != nullptr);

      arg->visit(visitor, mod);
    }
  }

  void AstCallStatement::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
    ASSERT(m_args.size() >= 1);

    auto &first_arg = m_args[0];
    ASSERT(first_arg != nullptr);

    first_arg->build(visitor, mod, out);
    visitor->getCompilationUnit()->getRegisterUsage().inc();

    for (int i = m_args.size() - 1; i >= 1; i--) {
      auto &arg = m_args[i];
      ASSERT(arg != nullptr);

      AstPushStatement(arg, arg->getLocation()).build(visitor, mod, out);

      visitor->getCompilationUnit()->getRelativeStackOffset().inc();
    }

    visitor->getCompilationUnit()->getRegisterUsage().dec();

    for (int i = 0; i < m_args.size() - 1; i++) {
      visitor->getCompilationUnit()->getRelativeStackOffset().dec();
    }

    out->append(std::unique_ptr<Op_Call>(new Op_Call(
      first_arg->getObjLoc()
    )));

    if (m_args.size() > 1) {
      AstPopStatement(m_args.size() - 1, m_location).build(visitor, mod, out);
    }
  }

  void AstCallStatement::optimize(AstVisitor *visitor, Module *mod) {
    for (auto &arg : m_args) {
      ASSERT(arg != nullptr);

      arg->optimize(visitor, mod);
    }
  }

  Pointer<AstStatement> AstCallStatement::clone() const {
    return CloneImpl();
  }
}
