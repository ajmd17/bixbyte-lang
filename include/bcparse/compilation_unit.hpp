#pragma once

#include <bcparse/error_list.hpp>
#include <bcparse/bound_variables.hpp>

#include <bcparse/emit/register_usage.hpp>
#include <bcparse/emit/relative_stack_offset.hpp>
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

    inline void setVariableMode(bool variableMode) { m_variableMode = variableMode; }
    inline bool isVariableMode() const { return m_variableMode; }

    inline ErrorList &getErrorList() { return m_errorList; }
    inline const ErrorList &getErrorList() const { return m_errorList; }

    inline BoundVariables &getBoundGlobals() { return m_boundGlobals; }
    inline const BoundVariables &getBoundGlobals() const { return m_boundGlobals; }

    inline RegisterUsage &getRegisterUsage() { return m_registerUsage; }
    inline const RegisterUsage &getRegisterUsage() const { return m_registerUsage; }

    inline RelativeStackOffset &getRelativeStackOffset() { return m_relativeStackOffset; }
    inline const RelativeStackOffset &getRelativeStackOffset() const { return m_relativeStackOffset; }

    inline DataStorage *getDataStorage() { return m_dataStorage; }
    inline const DataStorage *getDataStorage() const { return m_dataStorage; }

    private:
      ErrorList m_errorList;
      BoundVariables m_boundGlobals;
      RegisterUsage m_registerUsage;
      RelativeStackOffset m_relativeStackOffset;
      DataStorage *m_dataStorage;
      bool m_variableMode;
  };
}
