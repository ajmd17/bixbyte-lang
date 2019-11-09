#pragma once

#include <ostream>

namespace bcparse {
  class BytecodeChunk;
  class Formatter;

  class Emitter {
  public:
    Emitter(BytecodeChunk *chunk);

    void emit(std::ostream *os, Formatter *f);

  private:
    BytecodeChunk *m_chunk;
  };
}
