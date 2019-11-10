#include <bcparse/emit/buildable.hpp>
#include <bcparse/emit/bytecode_stream.hpp>
#include <bcparse/emit/formatter.hpp>

namespace bcparse {
  void Buildable::accept(BytecodeStream *bs) {
    m_loc = bs->streamOffset();
  }

  void Buildable::debugPrint(BytecodeStream *bs, Formatter *f) {
    f->setLineNo(m_loc);
  }
}
