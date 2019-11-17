#include <bcparse/ast/ast_interpolation.hpp>
#include <bcparse/ast/ast_variable.hpp>
#include <bcparse/ast/ast_symbol.hpp>

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
    m_container.owned = false;
    m_container.value = nullptr;
  }

  AstInterpolation::~AstInterpolation() {
    if (m_container.owned && m_container.value != nullptr) {
      delete m_container.value;

      m_container.value = nullptr;
      m_container.owned = false;
    }
  }

  void AstInterpolation::visit(AstVisitor *visitor, Module *mod) {
    AstCodeBody::visit(visitor, mod);

    if (m_iterator->size() != 0) {
      if (auto value = std::dynamic_pointer_cast<AstExpression>(m_iterator->last())) {
        setInterpValue(value.get(), false);
        setInterpValue(getInterpValue(), false);

        if (auto asVar = dynamic_cast<AstVariable*>(m_container.value)) {
          setInterpValue(asVar->getValueOf(), false);
        } else if (auto asSym = dynamic_cast<AstSymbol*>(m_container.value)) {
          setInterpValue(new AstVariable(asSym->getName(), m_location), true);

          m_container.value->visit(visitor, mod);
        }
      }
    }

    if (m_container.value == nullptr) {
      visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
        LEVEL_ERROR,
        Msg_custom_error,
        m_location,
        "Interpolation must be an expression"
      ));
    }
  }

  void AstInterpolation::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
    ASSERT(m_container.value != nullptr);

    m_container.value->build(visitor, mod, out);
    m_objLoc = m_container.value->getObjLoc();
  }

  Pointer<AstStatement> AstInterpolation::clone() const {
    return CloneImpl();
  }

  AstExpression *AstInterpolation::getValueOf() {
    if (m_container.value == nullptr) {
      return nullptr;
    }

    return m_container.value->getValueOf();
  }

  AstExpression *AstInterpolation::getDeepValueOf() {
    if (m_container.value == nullptr) {
      return nullptr;
    }

    return m_container.value->getDeepValueOf();
  }

  Value AstInterpolation::getRuntimeValue() const {
    if (m_container.value == nullptr) {
      return nullptr;
    }

    return m_container.value->getRuntimeValue();
  }

  std::string AstInterpolation::toString() const {
    if (m_container.value != nullptr) {
      return m_container.value->toString();
    }

    return AstExpression::toString();
  }

  AstExpression *AstInterpolation::getInterpValue() {
    if (auto asInterp = dynamic_cast<AstInterpolation*>(m_container.value)) {
      return asInterp->getInterpValue();
    }

    return m_container.value;
  }

  void AstInterpolation::setInterpValue(AstExpression *value, bool owned) {
    if (m_container.owned && m_container.value != nullptr) {
      delete m_container.value;
    }

    m_container.value = value;
    m_container.owned = owned;
  }
}
