#include <bcparse/ast/ast_print_statement.hpp>

#include <bcparse/ast_visitor.hpp>
#include <bcparse/compilation_unit.hpp>
#include <bcparse/emit/emit.hpp>

#include <common/my_assert.hpp>

namespace bcparse {
  AstPrintStatement::AstPrintStatement(std::vector<Pointer<AstExpression>> args,
    const SourceLocation &location)
    : AstStatement(location),
      m_args(args) {
  }

  void AstPrintStatement::visit(AstVisitor *visitor, Module *mod) {
    for (auto &arg : m_args) {
      ASSERT(arg != nullptr);

      arg->visit(visitor, mod);
    }
  }

  void AstPrintStatement::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
    for (auto &arg : m_args) {
      ASSERT(arg != nullptr);

      arg->build(visitor, mod, out);

      // if (arg->getObjLoc().getDataStoreLocation() == ObjLoc::DataStoreLocation::RegisterDataStore) {
      //   out->append(std::unique_ptr<Op_Load>(new Op_Load(
      //     arg->getObjLoc(),
      //     arg->getRuntimeValue()
      //   )));
      // }

      out->append(std::unique_ptr<Op_Print>(new Op_Print(
        arg->getObjLoc()
      )));
    }
  }

  void AstPrintStatement::optimize(AstVisitor *visitor, Module *mod) {
    for (auto &arg : m_args) {
      ASSERT(arg != nullptr);

      arg->optimize(visitor, mod);
    }
  }

  Pointer<AstStatement> AstPrintStatement::clone() const {
    return CloneImpl();
  }
}
