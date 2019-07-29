#include <bcparse/token.hpp>

#include <sstream>

namespace bcparse {
  const Token Token::EMPTY = Token(TK_EMPTY, "", SourceLocation::eof);

  std::string Token::tokenTypeToString(TokenClass tokenClass) {
    switch (tokenClass) {
      case TK_INTEGER:       return "integer";
      case TK_FLOAT:         return "float";
      case TK_STRING:        return "string";
      case TK_OPCODE:        return "opcode";
      case TK_REG:           return "register";
      case TK_LOCAL:         return "local";
      case TK_ADDR:          return "address";
      case TK_IDENT:         return "identifier";
      case TK_TAG:           return "tag";
      case TK_LABEL:         return "label";
      case TK_DIRECTIVE:     return "directive";
      case TK_INTERPOLATION: return "interpolation";
      case TK_NEWLINE:       return "\\n";
      case TK_COMMA:         return "comma";
      case TK_OPEN_PARENTH:  return "(";
      case TK_CLOSE_PARENTH: return ")";
      case TK_OPEN_BRACKET:  return "[";
      case TK_CLOSE_BRACKET: return "]";
      case TK_OPEN_BRACE:    return "{";
      case TK_CLOSE_BRACE:   return "}";
      default:               return "??";
    }
  }

  std::string Token::getRepr(const Token &token) {
    std::stringstream ss;

    switch (token.getTokenClass()) {
      case TK_STRING:
        ss << "\"" << token.getValue() << "\"";
        break;
      case TK_INTERPOLATION:
        ss << "#{" << token.getValue() << "}";
        break;
      case TK_REG:
        ss << "$R[" << token.getValue() << "]";
        break;
      case TK_LOCAL:
        ss << "$L[" << token.getValue() << "]";
        break;
      case TK_NEWLINE:       return "\n";
      default:               return token.getValue();
    }

    return ss.str();
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
