#include <bcparse/ast/directives/ast_debug_directive.hpp>

#include <bcparse/ast/ast_symbol.hpp>

#include <bcparse/ast_visitor.hpp>
#include <bcparse/compilation_unit.hpp>

namespace bcparse {

  AstDebugDirective::AstDebugDirective(const std::vector<Pointer<AstExpression>> &arguments,
    const std::vector<Token> &tokens,
    const SourceLocation &location)
    : AstDirectiveImpl(arguments, tokens, location) {
  }

  AstDebugDirective::~AstDebugDirective() {
  }

  void AstDebugDirective::visit(AstVisitor *visitor, Module *mod) {
    std::cout << "@debug says:  ";

    for (size_t i = 0; i < m_arguments.size(); i++) {
      auto arg = m_arguments[i];

      if (arg != nullptr) {
        arg->visit(visitor, mod);
      }

      std::cout << AstExpression::nodeToString(visitor, arg.get()) << "  ";
    }

    std::cout << "\n";
  }

  void AstDebugDirective::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
  }

  void AstDebugDirective::optimize(AstVisitor *visitor, Module *mod) {
  }
}
