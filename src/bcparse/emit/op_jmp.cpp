#include <bcparse/emit/emit.hpp>
#include <bcparse/emit/formatter.hpp>

#include <sstream>

namespace bcparse {
  Op_Jmp::Op_Jmp(const ObjLoc &objLoc, Flags flags)
    : m_objLoc(objLoc),
      m_flags(flags) {
  }

  void Op_Jmp::accept(BytecodeStream *bs) {
    Buildable::accept(bs);

    bs->acceptInstruction(0x5, (uint8_t)m_flags);
    bs->acceptObjLoc(m_objLoc);
  }

  void Op_Jmp::debugPrint(BytecodeStream *bs, Formatter *f) {
    Buildable::debugPrint(bs, f);

    std::stringstream ss;
    ss << "Op_Jmp("
       << m_objLoc.toString()
       << ", "
       << (uint32_t)m_flags
       << ")";

    f->append(ss.str());
  }
}
