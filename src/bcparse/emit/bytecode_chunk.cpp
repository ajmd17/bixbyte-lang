#include <bcparse/emit/bytecode_chunk.hpp>
#include <bcparse/emit/bytecode_stream.hpp>

namespace bcparse {
  BytecodeChunk::BytecodeChunk() {
  }

  void BytecodeChunk::append(std::unique_ptr<Buildable> buildable) {
    if (buildable != nullptr) {
      m_buildables.push_back(std::move(buildable));
    }
  }

  void BytecodeChunk::prepend(std::unique_ptr<Buildable> buildable) {
    if (buildable != nullptr) {
      m_buildables.push_front(std::move(buildable));
    }
  }

  void BytecodeChunk::accept(BytecodeStream *bs) {
    for (const auto &b : m_buildables) {
      b->accept(bs);
    }
  }

  LabelId_t BytecodeChunk::generateLabel() {
    LabelId_t id = m_labels.size();
    m_labels.emplace_back();
    return id;
  }
  
}
