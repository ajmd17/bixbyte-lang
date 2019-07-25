#pragma once

#include <bcparse/ast_visitor.hpp>

namespace bcparse {
  class Analyzer : public AstVisitor {
  public:
    Analyzer(AstIterator *iterator, CompilationUnit *compilationUnit);
    Analyzer(const Analyzer &other);

    void analyze();
  };
}
