#pragma once

#include <bcparse/macro.hpp>
#include <bcparse/error_list.hpp>
#include <bcparse/bound_variables.hpp>

#include <bcparse/emit/register_usage.hpp>
#include <bcparse/emit/data_storage.hpp>

#include <map>
#include <string>
#include <memory>

namespace bcparse {
  class CompilationUnit {
  public:
    CompilationUnit(DataStorage *dataStorage);
    CompilationUnit(const CompilationUnit &other) = delete;
    ~CompilationUnit();

    inline ErrorList &getErrorList() { return m_errorList; }
    inline const ErrorList &getErrorList() const { return m_errorList; }

    inline BoundVariables &getBoundGlobals() { return m_boundGlobals; }
    inline const BoundVariables &getBoundGlobals() const { return m_boundGlobals; }

    inline RegisterUsage &getRegisterUsage() { return m_registerUsage; }
    inline const RegisterUsage &getRegisterUsage() const { return m_registerUsage; }

    inline DataStorage *getDataStorage() { return m_dataStorage; }
    inline const DataStorage *getDataStorage() const { return m_dataStorage; }

    void defineMacro(const std::string &name, const std::string &body);
    Macro *lookupMacro(const std::string &name);

    private:
      ErrorList m_errorList;
      BoundVariables m_boundGlobals;
      RegisterUsage m_registerUsage;
      DataStorage *m_dataStorage;
      std::map<std::string, std::shared_ptr<Macro>> m_macros;
  };
}
