#include <bcparse/ast/ast_label.hpp>

#include <bcparse/ast_visitor.hpp>
#include <bcparse/compilation_unit.hpp>

#include <bcparse/emit/emit.hpp>

namespace bcparse {
  AstLabel::AstLabel(const std::string &name,
    const SourceLocation &location)
    : AstStatement(location),
      m_name(name) {
  }

  void AstLabel::visit(AstVisitor *visitor, Module *mod) {
  }

  void AstLabel::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
  }

  void AstLabel::optimize(AstVisitor *visitor, Module *mod) {
  }

  Pointer<AstStatement> AstLabel::clone() const {
    return CloneImpl();
  }
}
