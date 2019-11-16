#include <bcparse/ast/ast_label.hpp>
#include <bcparse/ast/ast_data_location.hpp>

#include <bcparse/ast_visitor.hpp>
#include <bcparse/compilation_unit.hpp>

#include <bcparse/emit/emit.hpp>

namespace bcparse {
  AstLabel::AstLabel(const std::string &name,
    const SourceLocation &location)
    : AstExpression(location),
      m_name(name),
      m_dataLocation(nullptr) {
  }

  void AstLabel::visit(AstVisitor *visitor, Module *mod) {

  }

  void AstLabel::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
    if (m_objLoc.getLocation() == -1) {
      size_t id = visitor->getCompilationUnit()->getDataStorage()->addLabel();

      m_dataLocation = std::shared_ptr<AstDataLocation>(new AstDataLocation(
        id,
        ObjLoc::DataStoreLocation::StaticDataStore,
        m_location
      ));

      m_objLoc = ObjLoc(id, ObjLoc::DataStoreLocation::StaticDataStore);
    }
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
    return this;
  }

  AstExpression *AstLabel::getDeepValueOf() {
    return this;
    if (m_dataLocation == nullptr) {
      return nullptr;
    }

    return m_dataLocation->getDeepValueOf();
  }
}
