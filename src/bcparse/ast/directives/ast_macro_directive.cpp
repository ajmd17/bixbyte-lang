#include <bcparse/ast/directives/ast_macro_directive.hpp>

#include <bcparse/ast/ast_symbol.hpp>

#include <bcparse/ast_visitor.hpp>
#include <bcparse/compilation_unit.hpp>

namespace bcparse {
  AstMacroDirective::AstMacroDirective(const std::vector<Pointer<AstExpression>> &arguments,
    const std::vector<Token> &tokens,
    const SourceLocation &location)
    : AstDirectiveImpl(arguments, tokens, location) {
  }

  AstMacroDirective::~AstMacroDirective() {
  }

  void AstMacroDirective::visit(AstVisitor *visitor, Module *mod) {
    const size_t numErrors = visitor->getCompilationUnit()->getErrorList().m_errors.size();

    AstSymbol *nameArg = nullptr;

    if (m_arguments.size() != 1) {
      visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
        LEVEL_ERROR,
        Msg_custom_error,
        m_location,
        "@macro requires arguments (name)"
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
        if (Macro *macro = visitor->getCompilationUnit()->getBoundGlobals().lookupMacro(nameArg->getName())) {
          visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
            LEVEL_ERROR,
            Msg_custom_error,
            m_location,
            std::string("@macro `") + nameArg->getName() + "` already defined"
          ));
        }
      }
    }

    if (m_tokens.empty()) {
      visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
        LEVEL_ERROR,
        Msg_custom_error,
        m_location,
        "@macro missing body"
      ));
    }

    if (visitor->getCompilationUnit()->getErrorList().m_errors.size() > numErrors) return;

    visitor->getCompilationUnit()->getBoundGlobals().defineMacro(nameArg->getName(), m_tokens);
  }

  void AstMacroDirective::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
  }

  void AstMacroDirective::optimize(AstVisitor *visitor, Module *mod) {
  }
}
