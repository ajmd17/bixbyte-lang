#include <bcparse/parser.hpp>

#include <bcparse/ast/ast_directive.hpp>

namespace bcparse {
  Parser::Parser(AstIterator *astIterator,
    TokenStream *tokenStream,
    CompilationUnit *compilationUnit)
    : m_astIterator(astIterator),
      m_tokenStream(tokenStream),
      m_compilationUnit(compilationUnit) {
  }

  Parser::Parser(const Parser &other)
    : m_astIterator(other.m_astIterator),
      m_tokenStream(other.m_tokenStream),
      m_compilationUnit(other.m_compilationUnit) {
  }

  Token Parser::match(Token::TokenClass tokenClass, bool read) {
    Token peek = m_tokenStream->peek();
    
    if (peek && peek.getTokenClass() == tokenClass) {
      if (read && m_tokenStream->hasNext()) {
        m_tokenStream->next();
      }
      
      return peek;
    }
    
    return Token::EMPTY;
  }

  Token Parser::matchAhead(Token::TokenClass tokenClass, int n) {
    Token peek = m_tokenStream->peek(n);
    
    if (peek && peek.getTokenClass() == tokenClass) {
      return peek;
    }
    
    return Token::EMPTY;
  }

  Token Parser::expect(Token::TokenClass tokenClass, bool read) {
    Token token = match(tokenClass, read);
    
    if (!token) {
      const SourceLocation location = currentLocation();

      ErrorMessage errorMsg;
      std::string errorStr;

      switch (tokenClass) {
        case Token::TK_IDENT:
          errorMsg = Msg_expected_identifier;
          break;
        default:
          errorMsg = Msg_expected_token;
          errorStr = Token::tokenTypeToString(tokenClass);
      }

      m_compilationUnit->getErrorList().addError(CompilerError(
        LEVEL_ERROR,
        errorMsg,
        location,
        errorStr
      ));
    }

    return token;
  }

  bool Parser::expectEndOfStmt() {
    const SourceLocation location = currentLocation();

    if (!match(Token::TK_NEWLINE, true)) {
      m_compilationUnit->getErrorList().addError(CompilerError(
        LEVEL_ERROR,
        Msg_expected_end_of_statement,
        location
      ));

      // skip until end of statement, end of line, or end of file.
      do {
        m_tokenStream->next();
      } while (m_tokenStream->hasNext() && !match(Token::TK_NEWLINE, true));

      return false;
    }

    return true;
  }

  SourceLocation Parser::currentLocation() const {
    if (m_tokenStream->getSize() != 0 && !m_tokenStream->hasNext()) {
      return m_tokenStream->last().getLocation();
    }

    return m_tokenStream->peek().getLocation();
  }

  void Parser::skipStatementTerminators() {
    // read past statement terminator tokens
    while (match(Token::TK_NEWLINE, true));
  }

  void Parser::parse() {
    skipStatementTerminators();

    while (m_tokenStream->hasNext()) {
      if (auto stmt = parseStatement()) {
        m_astIterator->push(stmt);
      } else {
        break;
      }
    }
  }

  Pointer<AstStatement> Parser::parseStatement() {
    Pointer<AstStatement> res;

    if (match(Token::TK_DIRECTIVE)) {
      res = parseDirective();
    }

    return res;
  }

  Pointer<AstExpression> Parser::parseTerm() {
    Token token = m_tokenStream->peek();

    if (!token) {
      m_compilationUnit->getErrorList().addError(CompilerError(
        LEVEL_ERROR,
        Msg_unexpected_eof,
        currentLocation()
      ));

      if (m_tokenStream->hasNext()) {
        m_tokenStream->next();
      }

      return nullptr;
    }

    Pointer<AstExpression> expr;

    if (match(Token::TK_OPEN_PARENTH)) {
      // expr = parseParentheses();
    } else if (match(Token::TK_OPEN_BRACKET)) {
      // expr = parseArrayExpression();
    } else if (match(Token::TK_INTEGER)) {
      // expr = parseIntegerLiteral();
    } else if (match(Token::TK_FLOAT)) {
      // expr = parseFloatLiteral();
    } else if (match(Token::TK_STRING)) {
      // expr = parseStringLiteral();
    } else {
      if (token.getTokenClass() == Token::TK_NEWLINE) {
        m_compilationUnit->getErrorList().addError(CompilerError(
          LEVEL_ERROR,
          Msg_unexpected_eol,
          token.getLocation()
        ));
      } else {
        m_compilationUnit->getErrorList().addError(CompilerError(
          LEVEL_ERROR,
          Msg_unexpected_token,
          token.getLocation(),
          token.getValue()
        ));
      }

      if (m_tokenStream->hasNext()) {
          m_tokenStream->next();
      }

      return nullptr;
    }

    return expr;
  }

  Pointer<AstExpression> Parser::parseExpression() {
    if (auto term = parseTerm()) {
      return term;
    }

    return nullptr;
  }

  Pointer<AstDirective> Parser::parseDirective() {
    if (Token token = expect(Token::TK_DIRECTIVE, true)) {
      std::vector<Pointer<AstExpression>> arguments;
      std::stringstream body;

      while (auto expr = parseExpression()) {
        arguments.emplace_back(expr);
      }

      if (match(Token::TK_OPEN_BRACE, true)) {
        int parenCounter = 1;

        while (m_tokenStream->hasNext()) {
          if (match(Token::TK_OPEN_BRACE)) {
            ++parenCounter;
          } else if (match(Token::TK_CLOSE_BRACE)) {
            --parenCounter;

            if (parenCounter == 0) {
              break;
            }
          }

          body << m_tokenStream->next().getValue();
        }
      }

      return Pointer<AstDirective>(new AstDirective(
        token.getValue(),
        arguments,
        body.str(),
        token.getLocation()
      ));
    }
  }
}
