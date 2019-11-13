#include <bcparse/ast/ast_integer_literal.hpp>

#include <bcparse/ast_visitor.hpp>
#include <bcparse/compilation_unit.hpp>

#include <bcparse/emit/emit.hpp>

#include <sstream>

namespace bcparse {
  AstIntegerLiteral::AstIntegerLiteral(int64_t value,
    const SourceLocation &location)
    : AstExpression(location),
      m_value(value) {
  }

  void AstIntegerLiteral::visit(AstVisitor *visitor, Module *mod) {
  }

  void AstIntegerLiteral::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
    m_objLoc = ObjLoc(
      visitor->getCompilationUnit()->getRegisterUsage().current(),
      ObjLoc::DataStoreLocation::RegisterDataStore
    );

    out->append(std::unique_ptr<Op_Load>(new Op_Load(
      m_objLoc,
      Value((int64_t)m_value)
    )));
  }

  void AstIntegerLiteral::optimize(AstVisitor *visitor, Module *mod) {
  }

  Pointer<AstStatement> AstIntegerLiteral::clone() const {
    return CloneImpl();
  }

  std::string AstIntegerLiteral::toString() const {
    std::stringstream ss;
    ss << m_value;

    return ss.str();
  }
}
