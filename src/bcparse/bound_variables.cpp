#include <bcparse/bound_variables.hpp>

#include <common/my_assert.hpp>

#include <stdexcept>

namespace bcparse {
  BoundVariables::BoundVariables()
    : m_parent(nullptr) {
  }

  BoundVariables::BoundVariables(const BoundVariables &other)
    : m_parent(other.m_parent),
      m_map(other.m_map),
      m_macros(other.m_macros) {
  }

  void BoundVariables::setParent(BoundVariables *parent) {
    ASSERT(parent != nullptr);

    if (m_parent != nullptr) {
      throw "parent already set";
    }

    m_parent = parent;
  }

  BoundVariables *BoundVariables::getParent() {
    return m_parent;
  }

  Pointer<AstExpression> BoundVariables::get(const std::string &name) {
    auto it = m_map.find(name);

    if (it == m_map.end()) {
      if (m_parent) {
        return m_parent->get(name);
      }

      return nullptr;
    }

    return it->second;
  }

  void BoundVariables::set(const std::string &name, const Pointer<AstExpression> &value) {
    m_map[name] = value;
  }

  void BoundVariables::defineMacro(const std::string &name, const std::string &body) {
    m_macros[name] = std::unique_ptr<Macro>(new Macro(name, body));
  }

  Macro *BoundVariables::lookupMacro(const std::string &name) {
    auto it = m_macros.find(name);

    if (it == m_macros.end()) {
      if (m_parent) {
        return m_parent->lookupMacro(name);
      }

      return nullptr;
    }

    return it->second.get();
  }
}
