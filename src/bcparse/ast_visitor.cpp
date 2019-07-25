#include <bcparse/ast_visitor.hpp>
#include <bcparse/compilation_unit.hpp>

namespace bcparse {
  AstVisitor::AstVisitor(AstIterator *iterator,
    CompilationUnit *compilationUnit)
    : m_iterator(iterator),
      m_compilationUnit(compilationUnit) {
  }

  bool AstVisitor::assertCond(bool cond, const CompilerError &error) {
    if (!cond) {
      m_compilationUnit->getErrorList().addError(error);
    }

    return cond;
  }
}
