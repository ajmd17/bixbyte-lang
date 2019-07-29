#pragma once

#include <cstdint>
#include <cstddef>

namespace bcparse {
  typedef std::uint32_t LabelPosition_t;
  typedef size_t LabelId_t;

  class BytecodeStream;

  struct LabelInfo {
    LabelPosition_t position;
  };

  class Buildable {
  public:
    virtual ~Buildable() = default;

    virtual void accept(BytecodeStream *bs) = 0;
  };
}
