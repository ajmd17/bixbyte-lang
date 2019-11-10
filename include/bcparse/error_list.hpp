#pragma once

#include <vector>
#include <algorithm>
#include <ostream>

#include <utf/utf8.hpp>
#include <bcparse/compiler_error.hpp>

namespace bcparse {
  class ErrorList {
  public:
    ErrorList();
    ErrorList(const ErrorList &other);

    inline void addError(const CompilerError &error) { m_errors.push_back(error); }
    inline void clearErrors() { m_errors.clear(); }
    inline void sortErrors() { std::sort(m_errors.begin(), m_errors.end()); }
    inline const std::vector<CompilerError> &getErrors() const { return m_errors; }

    bool hasFatalErrors() const;
    std::ostream &writeOutput(std::ostream &os) const; // @TODO make UTF8 compatible

    std::vector<CompilerError> m_errors;
  };
}
