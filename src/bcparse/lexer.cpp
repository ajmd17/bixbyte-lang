#include <bcparse/lexer.hpp>
#include <bcparse/token_stream.hpp>
#include <bcparse/compilation_unit.hpp>
#include <bcparse/source_file.hpp>

#include <common/my_assert.hpp>

#include <array>
#include <sstream>

using utf::u32char;
using UStr = utf::Utf8String;

namespace bcparse {
  Lexer::Lexer(const SourceStream &sourceStream,
    TokenStream *tokenStream,
    CompilationUnit *compilationUnit)
    : m_sourceStream(sourceStream),
      m_tokenStream(tokenStream),
      m_compilationUnit(compilationUnit),
      m_sourceLocation(0, 0, sourceStream.getFile()->getFilePath()) {
  }

  bool Lexer::expectChar(utf::u32char ch, bool read, int *posChange) {
    const SourceLocation location = m_sourceLocation;

    if (!m_sourceStream.hasNext()) {
      goto notFound;
    }

    if (m_sourceStream.peek() != ch) {
      goto notFound;
    }

    if (read) {
      ASSERT(posChange != nullptr);

      m_sourceStream.next(*posChange);

      return true;
    }

  notFound:
    m_compilationUnit->getErrorList().addError(CompilerError(
      LEVEL_ERROR,
      Msg_unexpected_token,
      location,
      std::string("") + (char)ch
    ));

    return false;
  }

  void Lexer::analyze() {
    // skip initial whitespace
    skipWhitespace();

    while (m_sourceStream.hasNext() && m_sourceStream.peek() != '\0') {
      Token token = nextToken();

      if (!token.empty()) {
        m_tokenStream->push(token);
      }

      // skipWhitespace() returns true if there was a newline
      const SourceLocation location = m_sourceLocation;

      if (skipWhitespace()) {
        // add the `newline` statement terminator if not a continuation token
        if (token && token.getTokenClass() != Token::TK_NEWLINE) {
          // skip whitespace before next token
          skipWhitespace();

          // check if next token is connected
          if (m_sourceStream.hasNext() && m_sourceStream.peek() != '\0') {
            auto peek = m_sourceStream.peek();

            if (peek == '{' || peek == '.') {
              // do not add newline
              continue;
            }
          }

          // add newline
          m_tokenStream->push(Token(Token::TK_NEWLINE, "\\n", location));
        }
      }
    }
  }

  bool Lexer::skipWhitespace() {
    bool hadNewline = false;

    while (m_sourceStream.hasNext() && utf::utf32_isspace(m_sourceStream.peek())) {
      int posChange = 0;

      if (m_sourceStream.next(posChange) == '\n') {
        m_sourceLocation.getLine()++;
        m_sourceLocation.getColumn() = 0;
        hadNewline = true;
      } else {
        m_sourceLocation.getColumn() += posChange;
      }
    }

    return hadNewline;
  }

  bool Lexer::hasNext() {
    if (!m_sourceStream.hasNext()) {
      m_compilationUnit->getErrorList().addError(CompilerError(
          LEVEL_ERROR,
          Msg_unexpected_eof,
          m_sourceLocation
      ));

      return false;
    }

    return true;
  }

  Token Lexer::nextToken() {
    SourceLocation location = m_sourceLocation;

    std::array<u32char, 3> ch;
    int totalPosChange = 0;
    for (int i = 0; i < 3; i++) {
      int posChange = 0;
      ch[i] = m_sourceStream.next(posChange);
      totalPosChange += posChange;
    }

    // go back to previous position
    m_sourceStream.goBack(totalPosChange);

    if (ch[0] == '\"' || ch[0] == '\'') {
      return readStringLiteral();
    } else if (ch[0] == '0' && (ch[1] == 'x' || ch[1] == 'X')) {
      return readHexNumberLiteral();
    } else if (utf::utf32_isdigit(ch[0]) || (ch[0] == '.' && utf::utf32_isdigit(ch[1]))) {
      return readNumberLiteral();
    } else if (ch[0] == '/' && ch[1] == '/') {
      return readComment();
    } else if (ch[0] == '@') {
      return readDirective();
    } else if (ch[0] == '#') {
      return readInterpolation();
    } else if (ch[0] == '_' || utf::utf32_isalpha(ch[0])) {
      return readIdentifier();
    } else if (ch[0] == '$') {
      return readDataLocation();
    } else if (ch[0] == ',') {
      int posChange = 0;
      m_sourceStream.next(posChange);
      m_sourceLocation.getColumn() += posChange;
      return Token(Token::TK_COMMA, ",", location);
    } else if (ch[0] == '(') {
      int posChange = 0;
      m_sourceStream.next(posChange);
      m_sourceLocation.getColumn() += posChange;
      return Token(Token::TK_OPEN_PARENTH, "(", location);
    } else if (ch[0] == ')') {
      int posChange = 0;
      m_sourceStream.next(posChange);
      m_sourceLocation.getColumn() += posChange;
      return Token(Token::TK_CLOSE_PARENTH, ")", location);
    } else if (ch[0] == '[') {
      int posChange = 0;
      m_sourceStream.next(posChange);
      m_sourceLocation.getColumn() += posChange;
      return Token(Token::TK_OPEN_BRACKET, "[", location);
    } else if (ch[0] == ']') {
      int posChange = 0;
      m_sourceStream.next(posChange);
      m_sourceLocation.getColumn() += posChange;
      return Token(Token::TK_CLOSE_BRACKET, "]", location);
    } else if (ch[0] == '{') {
      int posChange = 0;
      m_sourceStream.next(posChange);
      m_sourceLocation.getColumn() += posChange;
      return Token(Token::TK_OPEN_BRACE, "{", location);
    } else if (ch[0] == '}') {
      int posChange = 0;
      m_sourceStream.next(posChange);
      m_sourceLocation.getColumn() += posChange;
      return Token(Token::TK_CLOSE_BRACE, "}", location);
    } else {
      int posChange = 0;
      utf::u32char badToken = m_sourceStream.next(posChange);

      char badTokenStr[sizeof(badToken)] = { '\0' };
      utf::char32to8(badToken, badTokenStr);

      m_compilationUnit->getErrorList().addError(CompilerError(
        LEVEL_ERROR,
        Msg_unexpected_token,
        location,
        std::string(badTokenStr)
      ));

      m_sourceLocation.getColumn() += posChange;

      return Token::EMPTY;
    }
  }

  utf::u32char Lexer::readEscapeCode() {
    // location of the start of the escape code
    SourceLocation location = m_sourceLocation;

    if (hasNext()) {
      int posChange = 0;
      u32char esc = m_sourceStream.next(posChange);
      m_sourceLocation.getColumn() += posChange;

      // TODO: add support for unicode escapes
      switch (esc) {
      case 't': return '\t';
      case 'b': return '\b';
      case 'n': return '\n';
      case 'r': return '\r';
      case 'f': return '\f';
      case '\'':
      case '\"':
      case '\\':
        // return the escape itself
        return esc;
      default:
        m_compilationUnit->getErrorList().addError(CompilerError(
          LEVEL_ERROR,
          Msg_unrecognized_escape_sequence,
          location,
          std::string("\\") + utf::get_bytes(esc)
        ));
      }
    }

    return 0;
  }

  Token Lexer::readStringLiteral() {
    // the location for the start of the string
    SourceLocation location = m_sourceLocation;

    std::string value;
    int posChange = 0;

    u32char delim = m_sourceStream.next(posChange);
    m_sourceLocation.getColumn() += posChange;

    // the character as utf-32
    u32char ch = m_sourceStream.next(posChange);
    m_sourceLocation.getColumn() += posChange;

    while (ch != delim) {
      if (ch == (u32char)'\n' || !hasNext()) {
        // unterminated string literal
        m_compilationUnit->getErrorList().addError(CompilerError(
          LEVEL_ERROR,
          Msg_unterminated_string_literal,
          m_sourceLocation
        ));

        if (ch == (u32char)'\n') {
          // increment line and reset column
          m_sourceLocation.getColumn() = 0;
          m_sourceLocation.getLine()++;
        }

        break;
      }

      // determine whether to read an escape sequence
      if (ch == (u32char)'\\') {
        u32char esc = readEscapeCode();
        // append the bytes
        value.append(utf::get_bytes(esc));
      } else {
        // Append the character itself
        value.append(utf::get_bytes(ch));
      }

      ch = m_sourceStream.next(posChange);
      m_sourceLocation.getColumn() += posChange;
    }

    return Token(Token::TK_STRING, value, location);
  }

  Token Lexer::readNumberLiteral() {
    SourceLocation location = m_sourceLocation;

    // store the value in a string
    std::string value;

    // assume integer to start
    Token::TokenClass tokenClass = Token::TK_INTEGER;

    // allows support for floats starting with '.'
    if (m_sourceStream.peek() == '.') {
      tokenClass = Token::TK_FLOAT;
      value = "0.";
      int posChange = 0;
      m_sourceStream.next(posChange);
      m_sourceLocation.getColumn() += posChange;
    }

    u32char ch = m_sourceStream.peek();

    while (m_sourceStream.hasNext() && utf::utf32_isdigit(ch)) {
      int posChange = 0;
      u32char nextCh = m_sourceStream.next(posChange);
      value.append(utf::get_bytes(nextCh));
      m_sourceLocation.getColumn() += posChange;

      if (tokenClass != Token::TK_FLOAT) {
        if (m_sourceStream.hasNext()) {
            // the character as a utf-32 character
            u32char ch = m_sourceStream.peek();

            if (ch == (u32char)'.') {
              // read next to check if after is a digit
              int posChange = 0;
              m_sourceStream.next(posChange);

              u32char next = m_sourceStream.peek();

              if (!utf::utf32_isalpha(next) && next != (u32char)'_') {
                // type is a float because of '.' and not an identifier after
                tokenClass = Token::TK_FLOAT;
                value.append(utf::get_bytes(ch));
                m_sourceLocation.getColumn() += posChange;
              } else {
                // not a float literal, so go back on the '.'
                m_sourceStream.goBack(posChange);
              }
            }
        }
      }

      ch = m_sourceStream.peek();
    }

    return Token(tokenClass, value, location);
  }

  Token Lexer::readHexNumberLiteral() {
    // location of the start of the hex number
    SourceLocation location = m_sourceLocation;

    // store the value in a string
    std::string value;

    // read the "0x"
    for (int i = 0; i < 2; i++) {
      int posChange = 0;
      u32char next_ch = m_sourceStream.next(posChange);
      value.append(utf::get_bytes(next_ch));
      m_sourceLocation.getColumn() += posChange;
    }

    u32char ch = (u32char)('\0');

    do {
      int posChange = 0;
      u32char nextCh = m_sourceStream.next(posChange);
      value.append(utf::get_bytes(nextCh));
      m_sourceLocation.getColumn() += posChange;
      ch = m_sourceStream.peek();
    } while (std::isxdigit(ch));

    long num = std::strtol(value.c_str(), 0, 16);
    std::stringstream ss;
    ss << num;

    return Token(Token::TK_INTEGER, ss.str(), location);
  }

  Token Lexer::readComment() {
    SourceLocation location = m_sourceLocation;

    // read '//'
    for (int i = 0; i < 2; i++) {
      int posChange = 0;
      m_sourceStream.next(posChange);
      m_sourceLocation.getColumn() += posChange;
    }

    // read until newline or EOF is reached
    while (m_sourceStream.hasNext() && m_sourceStream.peek() != '\n') {
      int posChange = 0;
      m_sourceStream.next(posChange);
      m_sourceLocation.getColumn() += posChange;
    }

    return Token(Token::TK_NEWLINE, "\\n", location);
  }

  Token Lexer::readIdentifier() {
    SourceLocation location = m_sourceLocation;

    // store the name in this string
    std::string value;

    // the character as a utf-32 character
    u32char ch = m_sourceStream.peek();

  while (utf::utf32_isdigit(ch) || utf::utf32_isalpha(ch) || ch == '_' || ch == ':') {
      int posChange = 0;
      ch = m_sourceStream.next(posChange);
      m_sourceLocation.getColumn() += posChange;
      // append the raw bytes
      value.append(utf::get_bytes(ch));
      // set ch to be the next character in the buffer
      ch = m_sourceStream.peek();
    }

    Token::TokenClass tokenType = Token::TK_IDENT;

    if (value.back() == ':') {
      tokenType = Token::TK_LABEL;
      value.pop_back();
    }

    return Token(tokenType, value, location);
  }

  Token Lexer::readDirective() {
    SourceLocation location = m_sourceLocation;

    // read '@'
    int posChange = 0;
    m_sourceStream.next(posChange);
    m_sourceLocation.getColumn() += posChange;

    // store the name
    std::string value;

    // the character as a utf-32 character
    u32char ch = m_sourceStream.peek();

    while (utf::utf32_isdigit(ch) || ch == (u32char)('_') || utf::utf32_isalpha(ch)) {
      int posChange = 0;
      ch = m_sourceStream.next(posChange);
      m_sourceLocation.getColumn() += posChange;
      // append the raw bytes
      value.append(utf::get_bytes(ch));
      // set ch to be the next character in the buffer
      ch = m_sourceStream.peek();
    }

    return Token(Token::TK_DIRECTIVE, value, location);
  }

  Token Lexer::readDataLocation() {
    SourceLocation location = m_sourceLocation;

    int posChange;

    // read '$'
    m_sourceStream.next(posChange);
    m_sourceLocation.getColumn() += posChange;

    std::stringstream ss;

    Token ident = readIdentifier();
    ss << ident.getValue();

    if (m_sourceStream.peek() == '[') {
      m_sourceStream.next(posChange);
      m_sourceLocation.getColumn() += posChange;

      ss << '[';

      // the character as a utf-32 character
      u32char ch = m_sourceStream.peek();

      while (ch == '-' || ch == '+' || utf::utf32_isdigit(ch)) { // allow signs (negative = relative)
        int posChange = 0;
        ch = m_sourceStream.next(posChange);
        m_sourceLocation.getColumn() += posChange;
        // append the raw bytes
        ss << utf::get_bytes(ch);
        // set ch to be the next character in the buffer
        ch = m_sourceStream.peek();
      }

      if (expectChar(']', true, &posChange)) {
        ss << ']';
      }
    }

    return Token(Token::TK_DATA_LOC, ss.str(), location);
  }

  Token Lexer::readInterpolation() {
    SourceLocation location = m_sourceLocation;

    int posChange = 0;

    if (!expectChar('#', true, &posChange)) return Token::EMPTY;
    if (!expectChar('{', true, &posChange)) return Token::EMPTY;

    int parenCounter = 1;

    std::stringstream body;

    // read until newline or EOF is reached
    while (m_sourceStream.hasNext() && m_sourceStream.peek() != '\n') {
      int posChange = 0;
      utf::u32char ch = m_sourceStream.next(posChange);
      m_sourceLocation.getColumn() += posChange;

      switch (ch) {
      case '{':
        ++parenCounter;
        break;
      case '}':
        if (--parenCounter == 0) {
          return Token(Token::TK_INTERPOLATION, body.str(), location);
        }

        break;
      }

      body << (char)ch;
    }

    expectChar('}');

    return Token::EMPTY;
  }
}
