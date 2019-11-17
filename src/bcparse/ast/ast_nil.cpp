#include <bcparse/ast/ast_nil.hpp>

#include <bcparse/ast_visitor.hpp>
#include <bcparse/compilation_unit.hpp>

#include <bcparse/emit/emit.hpp>

#include <sstream>

namespace bcparse {
  AstNil::AstNil(const SourceLocation &location)
    : AstExpression(location) {
  }

  void AstNil::visit(AstVisitor *visitor, Module *mod) {
  }

  void AstNil::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
    m_objLoc = ObjLoc(
      visitor->getCompilationUnit()->getRegisterUsage().current(),
      ObjLoc::DataStoreLocation::RegisterDataStore
    );

    out->append(std::unique_ptr<Op_Load>(new Op_Load(
      m_objLoc,
      getRuntimeValue()
    )));
  }

  void AstNil::optimize(AstVisitor *visitor, Module *mod) {
  }

  Pointer<AstStatement> AstNil::clone() const {
    return CloneImpl();
  }

  std::string AstNil::toString() const {
    return "nil";
  }

  Value AstNil::getRuntimeValue() const {
    return Value(nullptr);
  }
}
