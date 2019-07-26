#pragma once

#include <bcparse/buildable.hpp>

#include <vector>
#include <memory>

namespace bcparse {
  class BytecodeChunk : public Buildable {
  public:
    BytecodeChunk();
    BytecodeChunk(const BytecodeChunk &other) = delete;
    virtual ~BytecodeChunk() = default;

    void append(const std::unique_ptr<Buildable> &buildable);
    LabelId_t generateLabel();

  private:
    std::vector<LabelInfo> m_labels;
    std::vector<std::unique_ptr<Buildable>> m_buildables;
  };
}
