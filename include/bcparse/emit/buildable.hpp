#pragma once

#include <cstdint>
#include <cstddef>

namespace bcparse {
  typedef std::uint32_t LabelPosition_t;
  typedef size_t LabelId_t;

  class BytecodeStream;
  class Formatter;

  struct LabelInfo {
    LabelPosition_t position;
  };

  class Buildable {
  public:
    virtual ~Buildable() = default;

    virtual void accept(BytecodeStream *bs);
    virtual void debugPrint(BytecodeStream *bs, Formatter *f);

  private:
    size_t m_loc;
  };
}
