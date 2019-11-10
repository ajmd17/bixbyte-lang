#pragma once

#include <bcparse/emit/obj_loc.hpp>

#include <vector>
#include <map>
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

    template <typename T> void acceptVector(const std::vector<T> &t) {
      m_data.insert(m_data.end(), t.begin(), t.end());
    }

    void acceptObjLoc(const ObjLoc &objLoc) {
      uint8_t at = (uint8_t)objLoc.getDataStoreLocation();
      at |= ((objLoc.getLocation() < 0 ? 0x8 : 0xC) << 2); // neg = relative
      uint32_t payload = (abs(objLoc.getLocation()) << 4) | (at & 0xF);

      acceptBytes(payload);
    }

    inline std::vector<uint8_t> &getData() { return m_data; }
    inline const std::vector<uint8_t> &getData() const { return m_data; }
    inline std::map<size_t, size_t> &getLabelOffsetMap() { return m_labelOffsetMap; }
    inline const std::map<size_t, size_t> &getLabelOffsetMap() const { return m_labelOffsetMap; }
    inline std::map<size_t, size_t> &getLabelAddressMap() { return m_labelAddressMap; }
    inline const std::map<size_t, size_t> &getLabelAddressMap() const { return m_labelAddressMap; }
    inline const size_t streamOffset() const { return m_data.size(); }

  private:
    std::vector<uint8_t> m_data;
    // map from label ID to starting index of uint64_t in m_data
    std::map<size_t, size_t> m_labelOffsetMap;
    // map from label ID to finalized address
    std::map<size_t, size_t> m_labelAddressMap;
  };
}
