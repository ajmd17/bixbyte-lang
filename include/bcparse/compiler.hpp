#pragma once

#include <bcparse/ast_visitor.hpp>

namespace bcparse {
  class BytecodeChunk;

  class Compiler : public AstVisitor {
  public:
    Compiler(AstIterator *iterator, CompilationUnit *compilationUnit);
    Compiler(const Compiler &other);

    void compile(BytecodeChunk *top);
  };
}
