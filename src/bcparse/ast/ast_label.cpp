#include <bcparse/ast/ast_label.hpp>

#include <bcparse/ast_visitor.hpp>
#include <bcparse/compilation_unit.hpp>

#include <bcparse/emit/emit.hpp>

namespace bcparse {
  AstLabel::AstLabel(const std::string &name,
    const SourceLocation &location)
    : AstExpression(location),
      m_name(name) {
  }

  void AstLabel::visit(AstVisitor *visitor, Module *mod) {
  }

  void AstLabel::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
    if (m_objLoc.getLocation() == -1) {
      size_t id = visitor->getCompilationUnit()->getDataStorage()->addLabel();

      m_objLoc = ObjLoc(id, ObjLoc::DataStoreLocation::StaticDataStore);
    }
  }

  void AstLabel::optimize(AstVisitor *visitor, Module *mod) {
  }

  Pointer<AstStatement> AstLabel::clone() const {
    return CloneImpl();
  }
}
