#include <bcparse/ast/ast_interpolation.hpp>
#include <bcparse/ast/ast_identifier.hpp>

#include <bcparse/ast_visitor.hpp>
#include <bcparse/compilation_unit.hpp>

#include <bcparse/emit/emit.hpp>

#include <common/my_assert.hpp>

namespace bcparse {
  AstInterpolation::AstInterpolation(Pointer<AstExpression> value,
    const SourceLocation &location)
    : AstExpression(location),
      m_value(value) {
  }

  void AstInterpolation::visit(AstVisitor *visitor, Module *mod) {
    ASSERT(m_value != nullptr);

    m_value->visit(visitor, mod);
  }

  void AstInterpolation::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
    ASSERT(m_value != nullptr);

    m_value->build(visitor, mod, out);

    m_objLoc = m_value->getObjLoc();
  }

  void AstInterpolation::optimize(AstVisitor *visitor, Module *mod) {
  }

  Pointer<AstStatement> AstInterpolation::clone() const {
    return CloneImpl();
  }

  AstExpression *AstInterpolation::getValueOf() {
    if (m_value == nullptr) {
      return nullptr;
    }

    AstExpression *expr = m_value->getValueOf();
    AstIdentifier *asIdent = dynamic_cast<AstIdentifier*>(expr);

    while (asIdent != nullptr) {
      expr = asIdent->getValue().get();
      asIdent = dynamic_cast<AstIdentifier*>(expr);
    }

    return expr;
  }
}
