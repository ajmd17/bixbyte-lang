#include <bcparse/emit/data_storage.hpp>
#include <bcparse/emit/emit.hpp>
#include <bcparse/emit/formatter.hpp>

#include <algorithm>

namespace bcparse {
  DataStorage::DataStorage() {
  }

  DataStorage::DataStorage(const DataStorage &other)
    : m_values(other.m_values) {
  }

  size_t DataStorage::addLabel() {
    size_t id = addStaticData(Value((uint64_t)0xFEEDC0DE), false /* do not cache placeholder value */);

    m_labelOffsets.insert(id);

    return id;
  }

  size_t DataStorage::addStaticData(const Value &value, bool cache) {
    if (cache) {
      auto it = std::find_if(m_values.begin(), m_values.end(), [&value](const Value &otherValue) {
        return value.getRawBytes() == otherValue.getRawBytes();
      });

      if (it != m_values.end()) {
        return it - m_values.begin();
      }
    }

    // add new value
    size_t id = m_values.size();
    m_values.push_back(value);

    return id;
  }

  void DataStorage::accept(BytecodeStream *bs) {
    Buildable::accept(bs);

    for (size_t i = 0; i < m_values.size(); i++) {
      bs->getLabelOffsetMap()[i] = bs->streamOffset();

      // @TODO assertion that it does not exceed max size
      m_opLoads.push_back(std::unique_ptr<Op_Load>(new Op_Load(
        ObjLoc(i, ObjLoc::DataStoreLocation::StaticDataStore),
        m_values[i]
      )));

      m_opLoads.back()->accept(bs);
    }
  }

  void DataStorage::debugPrint(BytecodeStream *bs, Formatter *f) {
    Buildable::debugPrint(bs, f);

    for (auto &opLoad : m_opLoads) {
      opLoad->debugPrint(bs, f);
    }
  }
}
