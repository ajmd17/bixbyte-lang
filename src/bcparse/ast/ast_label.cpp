#include <bcparse/ast/ast_label.hpp>
#include <bcparse/ast/ast_integer_literal.hpp>

#include <bcparse/ast_visitor.hpp>
#include <bcparse/compilation_unit.hpp>

#include <bcparse/emit/emit.hpp>

#include <common/my_assert.hpp>

namespace bcparse {
  AstLabel::AstLabel(const std::string &name,
    const Pointer<AstDataLocation> &dataLocation,
    const SourceLocation &location)
    : AstExpression(location),
      m_name(name),
      m_dataLocation(dataLocation) {
  }

  void AstLabel::visit(AstVisitor *visitor, Module *mod) {
    if (m_dataLocation == nullptr) {
      m_dataLocation = std::shared_ptr<AstDataLocation>(new AstDataLocation(
        "s", // static
        Pointer<AstIntegerLiteral>(new AstIntegerLiteral(
          visitor->getCompilationUnit()->getDataStorage()->addLabel(),
          m_location
        )),
        m_location
      ));
    }

    m_dataLocation->visit(visitor, mod);
  }

  void AstLabel::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
    ASSERT(m_dataLocation != nullptr);

    m_dataLocation->build(visitor, mod, out);
    m_objLoc = m_dataLocation->getObjLoc();
  }

  void AstLabel::optimize(AstVisitor *visitor, Module *mod) {
  }

  Pointer<AstStatement> AstLabel::clone() const {
    return CloneImpl();
  }

  std::string AstLabel::toString() const {
    return std::string("Label(") + m_name + ")";
  }

  AstExpression *AstLabel::getValueOf() {
    return m_dataLocation.get();
  }

  AstExpression *AstLabel::getDeepValueOf() {
    return this;
    if (m_dataLocation == nullptr) {
      return nullptr;
    }

    return m_dataLocation->getDeepValueOf();
  }
}
