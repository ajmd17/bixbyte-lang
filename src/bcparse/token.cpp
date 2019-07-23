#include <bcparse/token.hpp>

namespace bcparse {
  const Token Token::EMPTY = Token(TK_EMPTY, "", SourceLocation::eof);

  std::string Token::TokenTypeToString(TokenClass tokenClass) {
    switch (tokenClass) {
      case TK_INTEGER:       return "integer";
      case TK_FLOAT:         return "float";
      case TK_STRING:        return "string";
      case TK_OPCODE:        return "opcode";
      case TK_REG:           return "register";
      case TK_LOCAL:         return "local";
      case TK_ADDR:          return "address";
      default:               return "??";
    }
  }

  Token::Token(TokenClass tokenClass, const std::string &value, const SourceLocation &location)
      : m_tokenClass(tokenClass),
        m_value(value),
        m_location(location) {
  }

  Token::Token(const Token &other)
      : m_tokenClass(other.m_tokenClass),
        m_value(other.m_value),
        m_location(other.m_location) {
  }
}
