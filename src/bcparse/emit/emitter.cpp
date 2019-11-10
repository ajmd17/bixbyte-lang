#include <bcparse/emit/emitter.hpp>
#include <bcparse/emit/emit.hpp>
#include <bcparse/emit/formatter.hpp>

namespace bcparse {
  Emitter::Emitter(BytecodeChunk *chunk)
    : m_chunk(chunk) {
  }

  void Emitter::emit(std::ostream *os, Formatter *f) {
    BytecodeStream bs;
    Op_Halt op_halt;

    m_chunk->accept(&bs);
    op_halt.accept(&bs);

    if (f != nullptr) {
      f->setLineNo(0);

      m_chunk->debugPrint(&bs, f);
      op_halt.debugPrint(&bs, f);
    }

    os->write((char*)&bs.getData()[0], bs.getData().size());
  }
}
