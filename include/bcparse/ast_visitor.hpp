#pragma once

#include <bcparse/compiler_error.hpp>

namespace bcparse {
  class AstIterator;
  class CompilationUnit;

  class AstVisitor {
  public:
    AstVisitor(AstIterator *iterator,
      CompilationUnit *compilationUnit);

    inline AstIterator *getIterator() const { return m_iterator; }
    inline CompilationUnit *getCompilationUnit() const { return m_compilationUnit; }

    bool assertCond(bool cond, const CompilerError &error);

  protected:
    AstIterator *m_iterator;
    CompilationUnit *m_compilationUnit;
  };
}
