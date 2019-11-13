#include <bcparse/ast/ast_data_location.hpp>

#include <bcparse/ast_visitor.hpp>
#include <bcparse/compilation_unit.hpp>

#include <bcparse/emit/emit.hpp>

namespace bcparse {
  AstDataLocation::AstDataLocation(int value,
    ObjLoc::DataStoreLocation storagePath,
    const SourceLocation &location)
    : AstExpression(location),
      m_value(value),
      m_storagePath(storagePath) {
  }

  void AstDataLocation::visit(AstVisitor *visitor, Module *mod) {
  }

  void AstDataLocation::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
    m_objLoc = ObjLoc(m_value, m_storagePath);
  }

  void AstDataLocation::optimize(AstVisitor *visitor, Module *mod) {
  }

  Pointer<AstStatement> AstDataLocation::clone() const {
    return CloneImpl();
  }
}
