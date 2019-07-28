#include <bcparse/bytecode_chunk.hpp>

namespace bcparse {
  BytecodeChunk::BytecodeChunk() {
  }

  void BytecodeChunk::append(std::unique_ptr<Buildable> buildable) {
    if (buildable != nullptr) {
      m_buildables.push_back(std::move(buildable));
    }
  }

  LabelId_t BytecodeChunk::generateLabel() {
    LabelId_t id = m_labels.size();
    m_labels.emplace_back();
    return id;
  }
}
