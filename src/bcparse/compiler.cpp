#include <bcparse/compiler.hpp>
#include <bcparse/ast_iterator.hpp>
#include <bcparse/compilation_unit.hpp>

#include <common/my_assert.hpp>

namespace bcparse {
  Compiler::Compiler(AstIterator *iterator, CompilationUnit *compilationUnit)
    : Compiler(iterator, compilationUnit) {
  }

  Compiler::Compiler(const Compiler &other)
    : AstVisitor(other.m_iterator, other.m_compilationUnit) {
  }

  void Compiler::compile(BytecodeChunk *top) {
    while (m_iterator->hasNext()) {
      auto node = m_iterator->next();
      ASSERT(node != nullptr);

      BytecodeChunk *chunk = new BytecodeChunk;
      node->build(this, nullptr, chunk);
      top->append(std::unique_ptr<BytecodeChunk>(chunk));
    }
  }
}
