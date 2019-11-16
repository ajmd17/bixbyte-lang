#include <bcparse/ast/ast_interpolation.hpp>
#include <bcparse/ast/ast_variable.hpp>

#include <bcparse/ast_visitor.hpp>
#include <bcparse/ast_iterator.hpp>
#include <bcparse/compilation_unit.hpp>

#include <bcparse/emit/emit.hpp>

#include <common/my_assert.hpp>
#include <common/str_util.hpp>

#include <sstream>

namespace bcparse {
  AstInterpolation::AstInterpolation(const std::vector<Token> &tokens,
    const SourceLocation &location)
    : AstCodeBody(tokens, location, true) {
  }

  void AstInterpolation::visit(AstVisitor *visitor, Module *mod) {
    AstCodeBody::visit(visitor, mod);

    AstExpression *value = getValueOf();

    if (value == nullptr) {
      visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
        LEVEL_ERROR,
        Msg_custom_error,
        m_location,
        "Interpolation must be an expression"
      ));
    }
  }

  void AstInterpolation::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
    AstCodeBody::build(visitor, mod, out);

    if (AstExpression *value = getValueOf()) {
      m_objLoc = value->getObjLoc();
    }
  }

  Pointer<AstStatement> AstInterpolation::clone() const {
    return CloneImpl();
  }

  AstExpression *AstInterpolation::getValueOf() {
    ASSERT(m_iterator != nullptr);

    if (m_iterator->size() == 0) {
      return nullptr;
    }

    if (auto expr = std::dynamic_pointer_cast<AstExpression>(m_iterator->last())) {
      return expr.get();
    }

    return nullptr;
  }

  AstExpression *AstInterpolation::getDeepValueOf() {
    ASSERT(m_iterator != nullptr);

    if (m_iterator->size() == 0) {
      return nullptr;
    }

    if (auto expr = std::dynamic_pointer_cast<AstExpression>(m_iterator->last())) {
      return expr->getDeepValueOf();
    }

    return nullptr;
  }

  Value AstInterpolation::getRuntimeValue() const {
    ASSERT(m_iterator != nullptr);

    if (m_iterator->size() == 0) {
      return Value::none();
    }

    if (auto expr = std::dynamic_pointer_cast<AstExpression>(m_iterator->last())) {
      return expr->getRuntimeValue();
    }

    return nullptr;
  }

  std::string AstInterpolation::toString() const {
    ASSERT(m_iterator != nullptr);

    if (m_iterator->size() > 0) {
      if (const auto expr = (const_cast<AstInterpolation*>(this))->getValueOf()) {
        return expr->toString();
      }
    }

    return AstExpression::toString();
  }
}
