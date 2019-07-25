#include <bcparse/bound_variables.hpp>

#include <common/my_assert.hpp>

namespace bcparse {
  BoundVariables::BoundVariables()
    : m_parent(nullptr) {
  }

  BoundVariables::BoundVariables(const BoundVariables &other)
    : m_parent(other.m_parent),
      m_map(other.m_map) {
  }
  
  void setParent(BoundVariables *parent) {
    ASSERT(parent != nullptr);

    if (m_parent != nullptr) {
      throw std::exception("parent already set");
    }

    m_parent = parent;
  }

  BoundVariables *getParent() {
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
}
