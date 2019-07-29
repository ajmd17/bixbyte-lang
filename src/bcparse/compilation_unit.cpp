#include <bcparse/compilation_unit.hpp>

namespace bcparse {
  CompilationUnit::CompilationUnit(DataStorage *dataStorage)
    : m_dataStorage(dataStorage) {
  }

  CompilationUnit::~CompilationUnit() {
  }

  void CompilationUnit::defineMacro(const std::string &name, const std::string &body) {
    m_macros[name] = std::unique_ptr<Macro>(new Macro(name, body));
  }

  Macro *CompilationUnit::lookupMacro(const std::string &name) {
    auto it = m_macros.find(name);

    if (it == m_macros.end()) {
      return nullptr;
    }

    return it->second.get();
  }
}
