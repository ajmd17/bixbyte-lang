#pragma once

#include <memory>
#include <vector>

#include <bcparse/ast/ast_statement.hpp>

template <typename T>
using Pointer = std::shared_ptr<T>;

namespace bcparse {
  class AstIterator {
  public:
    AstIterator();
    AstIterator(const AstIterator &other);

    inline void push(const Pointer<AstStatement> &stmt) { m_list.push_back(stmt); }
    inline void pop() { m_list.pop_back(); }

    inline size_t getPosition() const { return m_position; }
    inline void setPosition(size_t position) { m_position = position; }
    inline void resetPosition() { m_position = 0; }

    inline Pointer<AstStatement> &peek() { return m_list[m_position]; }
    inline const Pointer<AstStatement> &peek() const { return m_list[m_position]; }
    inline Pointer<AstStatement> next() { return m_list[m_position++]; }
    inline bool hasNext() const { return m_position < m_list.size(); }

  private:
    size_t m_position;
    std::vector<Pointer<AstStatement>> m_list;
  };
}
