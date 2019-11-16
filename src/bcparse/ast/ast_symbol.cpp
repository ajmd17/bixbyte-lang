#if 1
#include <bcparse/ast/ast_symbol.hpp>
#include <bcparse/ast_visitor.hpp>
#include <bcparse/compilation_unit.hpp>

#include <bcparse/emit/emit.hpp>

#include <common/my_assert.hpp>

namespace bcparse {
  AstSymbol::AstSymbol(const std::string &name,
    const SourceLocation &location)
    : AstExpression(location),
      m_name(name) {
  }

  void AstSymbol::visit(AstVisitor *visitor, Module *mod) {
  }

  void AstSymbol::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
  }

  void AstSymbol::optimize(AstVisitor *visitor, Module *mod) {
  }

  Pointer<AstStatement> AstSymbol::clone() const {
    return CloneImpl();
  }
}
#endif
