#include <bcparse/emit/data_storage.hpp>
#include <bcparse/emit/emit.hpp>

#include <iostream>

namespace bcparse {
  DataStorage::DataStorage() {
  }

  DataStorage::DataStorage(const DataStorage &other)
    : m_values(other.m_values) {
  }

  size_t DataStorage::addStaticData(const Value &value) {
    size_t id = m_values.size();
    m_values.push_back(value);
    return id;
  }

  void DataStorage::accept(BytecodeStream *bs) {
    for (size_t i = 0; i < m_values.size(); i++) {
      // @TODO assertion that it does not exceed max size
      Op_Load(
        ObjLoc(i, ObjLoc::DataStoreLocation::StaticDataStore),
        m_values[i]
      ).accept(bs);
    }
  }
}
