#pragma once

#include <bcparse/error_list.hpp>

namespace bcparse {
  class CompilationUnit {
  public:
    CompilationUnit();
    CompilationUnit(const CompilationUnit &other) = delete;
    ~CompilationUnit();

    inline ErrorList &getErrorList() { return m_errorList; }
    inline const ErrorList &getErrorList() const { return m_errorList; }

    private:
      ErrorList m_errorList;
  };
}
