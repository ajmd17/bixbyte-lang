#include <bcparse/compiler.hpp>
#include <bcparse/ast_iterator.hpp>
#include <bcparse/compilation_unit.hpp>

#include <bcparse/emit/bytecode_chunk.hpp>

#include <common/my_assert.hpp>

namespace bcparse {
  Compiler::Compiler(AstIterator *iterator, CompilationUnit *compilationUnit)
    : AstVisitor(iterator, compilationUnit) {
  }

  Compiler::Compiler(const Compiler &other)
    : AstVisitor(other.m_iterator, other.m_compilationUnit) {
  }

  void Compiler::compile(BytecodeChunk *top, bool buildStaticData) {
    while (m_iterator->hasNext()) {
      auto node = m_iterator->next();
      ASSERT(node != nullptr);

      std::unique_ptr<BytecodeChunk> chunk(new BytecodeChunk);
      node->build(this, nullptr, chunk.get());
      top->append(std::move(chunk));
    }

    if (buildStaticData) {
      top->prepend(std::unique_ptr<DataStorage>(new DataStorage(
        *m_compilationUnit->getDataStorage()
      )));
    }
  }
}
