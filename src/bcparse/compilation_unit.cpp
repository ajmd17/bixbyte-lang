#include <bcparse/compilation_unit.hpp>

namespace bcparse {
  CompilationUnit::CompilationUnit(DataStorage *dataStorage)
    : m_dataStorage(dataStorage) {
  }

  CompilationUnit::~CompilationUnit() {
  }
}
