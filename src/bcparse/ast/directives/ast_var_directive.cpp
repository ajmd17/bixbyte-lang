#include <bcparse/ast/directives/ast_var_directive.hpp>

#include <bcparse/ast/ast_symbol.hpp>

#include <bcparse/ast_visitor.hpp>
#include <bcparse/compilation_unit.hpp>

namespace bcparse {
  AstVarDirective::AstVarDirective(const std::vector<Pointer<AstExpression>> &arguments,
    const std::vector<Token> &tokens,
    const SourceLocation &location)
    : AstDirectiveImpl(arguments, tokens, location) {
  }

  AstVarDirective::~AstVarDirective() {
  }

  void AstVarDirective::visit(AstVisitor *visitor, Module *mod) {
    AstSymbol *nameArg = nullptr;

    if (m_arguments.size() != 1) {
      visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
        LEVEL_ERROR,
        Msg_custom_error,
        m_location,
        "@var requires arguments (key)"
      ));
    } else {
      AstExpression *deepValue = nullptr;

      if (auto nameArgExpr = m_arguments[0].get()) {
        nameArgExpr->visit(visitor, mod);

        if ((deepValue = nameArgExpr->getDeepValueOf()) != nullptr) {
          nameArg = dynamic_cast<AstSymbol*>(deepValue);
        }
      }

      if (nameArg == nullptr) {
        if (m_arguments[0] != nullptr && deepValue != nullptr) {
          visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
            LEVEL_ERROR,
            Msg_custom_error,
            m_arguments[0]->getLocation(),
            "@set (key) must be an identifier, got %",
            AstExpression::nodeToString(visitor, deepValue)
          ));
        } else {
          visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
            LEVEL_ERROR,
            Msg_custom_error,
            m_location,
            "@set (key) must be an identifier"
          ));
        }
      } else {
        // set in current scope
        visitor->getCompilationUnit()->getBoundGlobals().set(
          nameArg->getName(), Pointer<AstUnset>(new AstUnset(m_location)));
      }
    }
  }

  void AstVarDirective::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
  }

  void AstVarDirective::optimize(AstVisitor *visitor, Module *mod) {
  }
}
