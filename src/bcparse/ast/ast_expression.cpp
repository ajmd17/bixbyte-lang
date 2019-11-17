#include <bcparse/ast/ast_expression.hpp>
#include <bcparse/ast/ast_symbol.hpp>
#include <bcparse/ast/ast_variable.hpp>

#include <bcparse/ast_visitor.hpp>
#include <bcparse/bound_variables.hpp>
#include <bcparse/compilation_unit.hpp>

#include <vector>
#include <map>
#include <sstream>

namespace bcparse {
  std::string AstExpression::nodeToString(AstVisitor *visitor, AstExpression *node) {
    std::stringstream ss;

    if (node == nullptr || node->getValueOf() == nullptr) {
      ss << "nullptr";
    } else if (auto asSym = dynamic_cast<AstSymbol*>(node->getValueOf())) {
      if (asSym->getName() == "vars") {
        // if currently in global scope, set the var as a global.
        // if not, bubble up to the scope highest enough to not reach global
        BoundVariables *root = &visitor->getCompilationUnit()->getBoundGlobals();
        std::vector<std::map<std::string, Pointer<AstExpression>>*> allBound;

        while (root->getParent() != nullptr) {
          allBound.push_back(&root->getMap());
          root = root->getParent();
        }

        std::reverse(allBound.begin(), allBound.end());

        for (size_t i = 0; i < allBound.size(); i++) {
          ss << "\n";

          for (auto it : *allBound[i]) {
            for (int j = 0; j < i; j++) {
              ss << "  ";
            }

            ss << it.first << ": " << nodeToString(visitor, it.second.get()) << "\n";
          }
        }
      } else {
        ss << "Symbol(" << asSym->getName() << ")";
      }
    }/* else if (auto asIdent = dynamic_cast<AstVariable*>(node->getValueOf())) {
      ss << nodeToString(visitor, asIdent->getValue().get());
    } */else {
      ss << node->toString();
    }

    return ss.str();
  }

  AstExpression::AstExpression(const SourceLocation &location)
    : AstStatement(location) {
  }

  AstExpression *AstExpression::getValueOf() {
    return this;
  }

  AstExpression *AstExpression::getDeepValueOf() {
    return getValueOf();
  }

  Value AstExpression::getRuntimeValue() const {
    return Value::none();
  }

  std::string AstExpression::toString() const {
    return typeid(*this).name();
  }
}
