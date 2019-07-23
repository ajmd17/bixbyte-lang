#include <bcparse/lexer.hpp>

using utf::u32char;

namespace bcparse {
  Lexer::Lexer(const SourceStream &sourceStream,
    TokenStream *tokenStream,
    CompilationUnit *compilationUnit)
    : m_sourceStream(sourceStream),
      m_tokenStream(tokenStream),
      m_compilationUnit(compilationUnit),
      m_sourceLocation(0, 0, sourceStream.getFile()->getFilePath()) {
  }

  void Lexer::analyze() {

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

  }

  Token Lexer::readComment() {

  }

  Token Lexer::readIdentifier() {

  }
}
