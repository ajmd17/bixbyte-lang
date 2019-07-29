#pragma once

#include <bcparse/emit/buildable.hpp>
#include <bcparse/emit/value.hpp>

#include <vector>

namespace bcparse {
  class DataStorage : public Buildable {
  public:
    DataStorage();
    DataStorage(const DataStorage &other);
    virtual ~DataStorage() = default;

    size_t addStaticData(const Value &value); // returns index/id
    size_t getSize() const { return m_values.size(); }

    virtual void accept(BytecodeStream *bs) override;

  private:
    std::vector<Value> m_values;
  };
}
