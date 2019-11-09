#pragma once

#include <bcparse/emit/buildable.hpp>

#include <vector>
#include <deque>
#include <memory>

namespace bcparse {
  class BytecodeStream;

  class BytecodeChunk : public Buildable {
  public:
    BytecodeChunk();
    BytecodeChunk(const BytecodeChunk &other) = delete;
    virtual ~BytecodeChunk() = default;

    void append(std::unique_ptr<Buildable> buildable);
    void prepend(std::unique_ptr<Buildable> buildable);
    void accept(BytecodeStream *bs) override;
    void debugPrint(Formatter *f) override;

    LabelId_t generateLabel();

  private:
    std::vector<LabelInfo> m_labels;
    std::deque<std::unique_ptr<Buildable>> m_buildables;
  };
}
