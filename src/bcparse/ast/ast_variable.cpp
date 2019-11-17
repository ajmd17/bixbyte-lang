#include <bcparse/ast/ast_variable.hpp>
#include <bcparse/ast/ast_unset.hpp>

#include <bcparse/ast_visitor.hpp>
#include <bcparse/compilation_unit.hpp>

#include <bcparse/emit/emit.hpp>

#include <common/my_assert.hpp>

#include <sstream>

namespace bcparse {
  AstVariable::AstVariable(const std::string &name,
    const SourceLocation &location)
    : AstExpression(location),
      m_name(name),
      m_value(nullptr) {
  }

  void AstVariable::visit(AstVisitor *visitor, Module *mod) {
    // if (!visitor->getCompilationUnit()->isVariableMode()) {
    //   return;
    // }

    if (auto ptr = visitor->getCompilationUnit()->getBoundGlobals().get(m_name)) {
      m_value = ptr;

      // put it in this class as well so that error messages
      // display the correct usage location
      if (dynamic_cast<AstUnset*>(m_value.get())) {
        visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
          LEVEL_ERROR,
          Msg_custom_error,
          m_location,
          "unset var used"
        ));
      } else {
        m_value->visit(visitor, mod);
      }

      return;
    }

    visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
      LEVEL_ERROR,
      Msg_undeclared_identifier,
      m_location,
      m_name
    ));
  }

  void AstVariable::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
    // if (visitor->getCompilationUnit()->isVariableMode()) {
      ASSERT(m_value != nullptr);

      m_value->build(visitor, mod, out);

      m_objLoc = m_value->getObjLoc();
    // }
  }

  void AstVariable::optimize(AstVisitor *visitor, Module *mod) {
    // TODO: inlining?
  }

  Pointer<AstStatement> AstVariable::clone() const {
    return CloneImpl();
  }

  AstExpression *AstVariable::getValueOf() {
    // return this
    if (m_value == nullptr) {
      return this;
    }

    return m_value->getValueOf();
  }

  Value AstVariable::getRuntimeValue() const {
    if (m_value == nullptr) {
      return Value::none();
    }

    return m_value->getRuntimeValue();
  }

  std::string AstVariable::toString() const {
    std::stringstream ss;

    ss << "Var(";
    ss << m_name;
    ss << ": ";

    if (m_value != nullptr) {
      ss << m_value->toString();
    } else {
      ss << "unset";
    }

    ss << ")";

    return ss.str();
  }
}
