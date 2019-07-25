#pragma once

#include <bcparse/error_list.hpp>
#include <bcparse/bound_variables.hpp>

namespace bcparse {
  class CompilationUnit {
  public:
    CompilationUnit();
    CompilationUnit(const CompilationUnit &other) = delete;
    ~CompilationUnit();

    inline ErrorList &getErrorList() { return m_errorList; }
    inline const ErrorList &getErrorList() const { return m_errorList; }

    inline BoundVariables &getBoundGlobals() { return m_boundGlobals; }
    inline const BoundVariables &getBoundGlobals() const { return m_boundGlobals; }

    private:
      ErrorList m_errorList;
      BoundVariables m_boundGlobals;
  };
}
