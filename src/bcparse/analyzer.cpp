#include <bcparse/analyzer.hpp>
#include <bcparse/ast_iterator.hpp>
#include <bcparse/compilation_unit.hpp>

#include <common/my_assert.hpp>

namespace bcparse {
  Analyzer::Analyzer(AstIterator *iterator, CompilationUnit *compilationUnit)
    : AstVisitor(iterator, compilationUnit) {
  }

  Analyzer::Analyzer(const Analyzer &other)
    : AstVisitor(other.m_iterator, other.m_compilationUnit) {
  }

  void Analyzer::analyze() {
    while (m_iterator->hasNext()) {
      auto node = m_iterator->next();
      ASSERT(node != nullptr);

      node->visit(this, nullptr); // @TODO module
    }
  }
}
