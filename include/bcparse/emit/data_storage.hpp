#pragma once

#include <bcparse/emit/buildable.hpp>
#include <bcparse/emit/value.hpp>
#include <bcparse/emit/emit.hpp>

#include <vector>
#include <set>
#include <memory>

namespace bcparse {
  class DataStorage : public Buildable {
  public:
    DataStorage();
    DataStorage(const DataStorage &other);
    virtual ~DataStorage() = default;

    inline std::vector<Value> &getValues() { return m_values; }
    inline const std::vector<Value> &getValues() const { return m_values; }

    size_t addLabel(); // returns index/id
    size_t addStaticData(const Value &value, bool cache = true); // returns index/id
    size_t getSize() const { return m_values.size(); }

    virtual void accept(BytecodeStream *bs) override;
    virtual void debugPrint(BytecodeStream *bs, Formatter *f) override;

  private:
    std::vector<Value> m_values;
    std::set<size_t> m_labelOffsets; // vector of indices of m_values.

    std::vector<std::unique_ptr<Op_Load>> m_opLoads;
  };
}
