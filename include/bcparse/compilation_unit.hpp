#pragma once

#include <bcparse/error_list.hpp>
#include <bcparse/bound_variables.hpp>
#include <bcparse/macro.hpp>

#include <map>
#include <string>
#include <memory>

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

    void defineMacro(const std::string &name, const std::string &body);
    Macro *lookupMacro(const std::string &name);

    private:
      ErrorList m_errorList;
      BoundVariables m_boundGlobals;
      std::map<std::string, std::shared_ptr<Macro>> m_macros;
  };
}
