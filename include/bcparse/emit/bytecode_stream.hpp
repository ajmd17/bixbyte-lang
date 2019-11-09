#pragma once

#include <bcparse/emit/obj_loc.hpp>

#include <vector>
#include <cstring>

namespace bcparse {
  class BytecodeStream {
  public:
    void acceptString(const char *str) {
      // do not copy NUL byte
      size_t length = std::strlen(str);
      m_data.insert(m_data.end(), str, str + length);
    }

    void acceptInstruction(uint8_t opcode, uint8_t flags = 0) {
      uint8_t payload = opcode;
      payload <<= 3;
      payload |= flags;
      acceptBytes(payload);
    }

    template <typename T> void acceptBytes(const T &t) {
      char bytes[sizeof(t)];
      std::memcpy(&bytes[0], &t, sizeof(t));
      m_data.insert(m_data.end(), &bytes[0], &bytes[0] + sizeof(t));
    }

    void acceptObjLoc(const ObjLoc &objLoc) {
      uint8_t at = (uint8_t)objLoc.getDataStoreLocation();
      at |= ((objLoc.getLocation() < 0 ? 0x8 : 0xC) << 2); // neg = relative
      uint32_t payload = (abs(objLoc.getLocation()) << 4) | (at & 0xF);

      acceptBytes(payload);
    }

    inline const std::vector<uint8_t> &getData() const { return m_data; }

  private:
    std::vector<uint8_t> m_data;
  };
}
