#include <bcparse/ast/directives/ast_set_directive.hpp>

#include <bcparse/ast/ast_symbol.hpp>

#include <bcparse/ast_visitor.hpp>
#include <bcparse/compilation_unit.hpp>

namespace bcparse {
  AstSetDirective::AstSetDirective(const std::vector<Pointer<AstExpression>> &arguments,
    const std::vector<Token> &tokens,
    const SourceLocation &location)
    : AstDirectiveImpl(arguments, tokens, location) {
  }

  AstSetDirective::~AstSetDirective() {
  }

  void AstSetDirective::visit(AstVisitor *visitor, Module *mod) {
    AstSymbol *nameArg = nullptr;
    AstExpression *valueArg = nullptr;

    if (m_arguments.size() != 2) {
      visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
        LEVEL_ERROR,
        Msg_custom_error,
        m_location,
        "@set requires arguments (key, value)"
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
      }

      if ((valueArg = m_arguments[1].get())) {
        valueArg->visit(visitor, mod);

        if (valueArg->getDeepValueOf() != nullptr) {
          valueArg = valueArg->getDeepValueOf();
        }
      }

      if (valueArg == nullptr) {
        visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
          LEVEL_ERROR,
          Msg_custom_error,
          m_location,
          "@set (value) must be an expression"
        ));
      }

      if (nameArg != nullptr && valueArg != nullptr) {
        // if currently in global scope, set the var as a global.
        // if not, bubble up to the scope highest enough to not reach global
        Pointer<AstExpression> var;
        BoundVariables *boundVariables = &visitor->getCompilationUnit()->getBoundGlobals();
        BoundVariables *varScope = boundVariables;

        // set in closest @var scope or in global scope if none found
        while (true) {
          if (var == nullptr) {
            var = boundVariables->get(nameArg->getName(), false);

            varScope = boundVariables;
          }

          boundVariables = boundVariables->getParent();

          if (boundVariables == nullptr) {
            break;
          }
        }

        Pointer<AstExpression> clonedExpr = std::dynamic_pointer_cast<AstExpression>(valueArg->clone());
        varScope->set(nameArg->getName(), clonedExpr);
      }
    }
  }

  void AstSetDirective::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
  }

  void AstSetDirective::optimize(AstVisitor *visitor, Module *mod) {
  }
}
