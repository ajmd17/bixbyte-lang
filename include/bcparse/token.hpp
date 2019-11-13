#pragma once

#include <string>

#include <bcparse/source_location.hpp>

namespace bcparse {
  class Token {
  public:
    enum TokenClass {
      TK_EMPTY,
      TK_INTEGER,
      TK_FLOAT,
      TK_STRING,
      TK_OPCODE,
      TK_REG,
      TK_LOCAL,
      TK_ADDR,
      TK_IDENT,
      TK_TAG,
      TK_LABEL,
      TK_DIRECTIVE,
      TK_INTERPOLATION,
      TK_NEWLINE,
      TK_COMMA,
      TK_OPEN_PARENTH,
      TK_CLOSE_PARENTH,
      TK_OPEN_BRACKET,
      TK_CLOSE_BRACKET,
      TK_OPEN_BRACE,
      TK_CLOSE_BRACE
    };

    static std::string tokenTypeToString(TokenClass tokenClass);
    static std::string getRepr(const Token &token);
    static const Token EMPTY;

    Token(TokenClass tokenClass,
      const std::string &value,
      const SourceLocation &location);

    Token(const Token &other);

    inline TokenClass getTokenClass() const { return m_tokenClass; }
    inline const std::string &getValue() const { return m_value; }
    inline const SourceLocation &getLocation() const { return m_location; }
    inline bool empty() const { return m_tokenClass == TK_EMPTY; }

    inline Token &operator=(const Token &other) {
      m_tokenClass = other.m_tokenClass;
      m_value = other.m_value;
      m_location = other.m_location;

      return *this;
    }

    // return true if not empty
    inline explicit operator bool() const { return m_tokenClass != TK_EMPTY; }

  private:
    TokenClass m_tokenClass;
    std::string m_value;
    SourceLocation m_location;
  };
}
