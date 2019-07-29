#pragma once

#include <ostream>

namespace bcparse {
  class BytecodeChunk;

  class Emitter {
  public:
    Emitter(BytecodeChunk *chunk);

    void emit(std::ostream *os);

  private:
    BytecodeChunk *m_chunk;
  };
}
