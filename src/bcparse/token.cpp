#include <bcparse/token.hpp>

#include <sstream>

namespace bcparse {
  const Token Token::EMPTY = Token(TK_EMPTY, "", SourceLocation::eof);

  std::string Token::tokenTypeToString(TokenClass tokenClass) {
    switch (tokenClass) {
      case TK_INTEGER:       return "TK_INTEGER";
      case TK_FLOAT:         return "TK_FLOAT";
      case TK_STRING:        return "TK_STRING";
      case TK_OPCODE:        return "TK_OPCODE";
      case TK_DATA_LOC:      return "TK_DATA_LOC";
      case TK_ADDR:          return "TK_ADDR";
      case TK_IDENT:         return "TK_IDENT";
      case TK_TAG:           return "TK_TAG";
      case TK_LABEL:         return "TK_LABEL";
      case TK_DIRECTIVE:     return "TK_DIRECTIVE";
      case TK_INTERPOLATION: return "TK_INTERPOLATION";
      case TK_NEWLINE:       return "TK_NEWLINE";
      case TK_COMMA:         return "TK_COMMA";
      case TK_OPEN_PARENTH:  return "TK_OPEN_PARENTH";
      case TK_CLOSE_PARENTH: return "TK_CLOSE_PARENTH";
      case TK_OPEN_BRACKET:  return "TK_OPEN_BRACKET";
      case TK_CLOSE_BRACKET: return "TK_CLOSE_BRACKET";
      case TK_OPEN_BRACE:    return "TK_OPEN_BRACE";
      case TK_CLOSE_BRACE:   return "TK_CLOSE_BRACE";
      default:               return "??";
    }
  }

  std::string Token::getRepr(const Token &token) {
    std::stringstream ss;

    switch (token.getTokenClass()) {
      case TK_STRING:
        ss << "\"" << token.getValue() << "\"";
        break;
      case TK_LABEL:
        ss << token.getValue() << ":";
        break;
      case TK_DIRECTIVE:
        ss << "@" << token.getValue();
        break;
      case TK_INTERPOLATION:
        ss << "#{" << token.getValue() << "}";
        break;
      case TK_DATA_LOC:
        ss << "$" << token.getValue();
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
