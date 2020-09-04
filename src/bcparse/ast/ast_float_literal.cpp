#include <bcparse/ast/ast_float_literal.hpp>

#include <bcparse/ast_visitor.hpp>
#include <bcparse/compilation_unit.hpp>

#include <bcparse/emit/emit.hpp>

#include <sstream>

namespace bcparse {
  AstFloatLiteral::AstFloatLiteral(double value,
    const SourceLocation &location)
    : AstExpression(location),
      m_value(value) {
  }

  void AstFloatLiteral::visit(AstVisitor *visitor, Module *mod) {
  }

  void AstFloatLiteral::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
    m_objLoc = ObjLoc(
      visitor->getCompilationUnit()->getRegisterUsage().current(),
      ObjLoc::DataStoreLocation::RegisterDataStore
    );

    out->append(std::unique_ptr<Op_Load>(new Op_Load(
      m_objLoc,
      getRuntimeValue()
    )));
  }

  void AstFloatLiteral::optimize(AstVisitor *visitor, Module *mod) {
  }

  Pointer<AstStatement> AstFloatLiteral::clone() const {
    return CloneImpl();
  }

  std::string AstFloatLiteral::toString() const {
    std::stringstream ss;
    ss << m_value;

    return ss.str();
  }

  Value AstFloatLiteral::getRuntimeValue() const {
    return Value((double)m_value);
  }
}
