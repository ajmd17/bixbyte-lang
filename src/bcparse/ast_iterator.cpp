#include <bcparse/ast_iterator.hpp>

namespace bcparse {
  AstIterator::AstIterator()
    : m_position(0) {
  }

  AstIterator::AstIterator(const AstIterator &other)
    : m_position(other.m_position),
      m_list(other.m_list) {
  }
}
