#pragma once

namespace bcparse {
  class ObjLoc {
  public:
    enum class DataStoreLocation {
      StaticDataStore = 0x1,
      LocalDataStore = 0x2,
      RegisterDataStore = 0x3
    };

    ObjLoc(int location, DataStoreLocation dataStoreLocation)
      : m_location(location),
        m_dataStoreLocation(dataStoreLocation) {
    }

    ObjLoc(const ObjLoc &other)
      : m_location(other.m_location),
        m_dataStoreLocation(other.m_dataStoreLocation) {
    }

    inline int getLocation() const { return m_location; }
    inline DataStoreLocation getDataStoreLocation() const { return m_dataStoreLocation; }

  private:
    int m_location;
    DataStoreLocation m_dataStoreLocation;
  };
}
