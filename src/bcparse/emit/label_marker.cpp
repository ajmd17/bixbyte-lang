#include <bcparse/emit/emit.hpp>
#include <bcparse/emit/formatter.hpp>

#include <sstream>

namespace bcparse {
  LabelMarker::LabelMarker(size_t labelId)
    : m_labelId(labelId),
      m_opLoad(nullptr) {
  }

  LabelMarker::~LabelMarker() {
    if (m_opLoad != nullptr) {
      delete m_opLoad;
      m_opLoad = nullptr;
    }
  }

  void LabelMarker::accept(BytecodeStream *bs) {
    Buildable::accept(bs);

    const uint64_t address = bs->streamOffset();
    const size_t offset = bs->getLabelOffsetMap()[m_labelId];

    bs->getLabelAddressMap()[m_labelId] = address;

    // create sub-bytecode stream then overwrite data at `loc`
    BytecodeStream sub;

    m_opLoad = new Op_Load(
      ObjLoc(m_labelId, ObjLoc::DataStoreLocation::StaticDataStore),
      Value(address)
    );

    // load current stream offset into static data store
    m_opLoad->accept(&sub);

    for (size_t i = 0; i < sub.getData().size(); i++) {
      bs->getData()[offset + i] = sub.getData()[i];
    }
  }

  void LabelMarker::debugPrint(BytecodeStream *bs, Formatter *f) {
    // Buildable::debugPrint(bs, f);

    // if (m_opLoad != nullptr) {
    //   m_opLoad->debugPrint(bs, f);
    // }
  }
}
