#include <bcparse/emit/emitter.hpp>
#include <bcparse/emit/emit.hpp>

namespace bcparse {
  Emitter::Emitter(BytecodeChunk *chunk)
    : m_chunk(chunk) {  
  }

  void Emitter::emit(std::ostream *os) {
    BytecodeStream bs;

    m_chunk->accept(&bs);
    Op_Halt().accept(&bs);

    os->write((char*)&bs.getData()[0], bs.getData().size());
  }
}
