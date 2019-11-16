#include <bcparse/ast/ast_push_statement.hpp>
#include <bcparse/ast/ast_data_location.hpp>

#include <bcparse/ast_visitor.hpp>
#include <bcparse/compilation_unit.hpp>
#include <bcparse/emit/emit.hpp>

#include <common/my_assert.hpp>

namespace bcparse {
  AstPushStatement::AstPushStatement(Pointer<AstExpression> arg,
    const SourceLocation &location)
    : AstStatement(location),
      m_arg(arg) {
  }

  void AstPushStatement::visit(AstVisitor *visitor, Module *mod) {
    ASSERT(m_arg != nullptr);

    m_arg->visit(visitor, mod);
  }

  void AstPushStatement::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
    ASSERT(m_arg != nullptr);

    m_arg->build(visitor, mod, out);

    AstExpression *deepValue = m_arg->getDeepValueOf();

    ASSERT(deepValue != nullptr);

    if (auto asDataLocation = dynamic_cast<AstDataLocation*>(deepValue)) {
      out->append(std::unique_ptr<Op_Push>(new Op_Push(
        asDataLocation->getObjLoc()
      )));

      return;
    }

    out->append(std::unique_ptr<Op_PushConst>(new Op_PushConst(
      m_arg->getRuntimeValue()
    )));
  }

  void AstPushStatement::optimize(AstVisitor *visitor, Module *mod) {
    ASSERT(m_arg != nullptr);

    m_arg->optimize(visitor, mod);
  }

  Pointer<AstStatement> AstPushStatement::clone() const {
    return CloneImpl();
  }
}
