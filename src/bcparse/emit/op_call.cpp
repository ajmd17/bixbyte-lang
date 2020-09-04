#include <bcparse/emit/emit.hpp>
#include <bcparse/emit/formatter.hpp>

#include <sstream>

namespace bcparse {
  Op_Call::Op_Call(const ObjLoc &objLoc)
    : m_objLoc(objLoc) {
  }

  void Op_Call::accept(BytecodeStream *bs) {
    Buildable::accept(bs);

    bs->acceptInstruction(0x14, 0);
    bs->acceptObjLoc(m_objLoc);
  }

  void Op_Call::debugPrint(BytecodeStream *bs, Formatter *f) {
    Buildable::debugPrint(bs, f);

    std::stringstream ss;
    ss << "Op_Call("
       << m_objLoc.toString()
       << ")";

    f->append(ss.str());
  }
}
