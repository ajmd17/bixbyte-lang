#include <bcparse/parser.hpp>
#include <bcparse/lexer.hpp>
#include <bcparse/analyzer.hpp>
#include <bcparse/source_file.hpp>
#include <bcparse/source_stream.hpp>

#include <bcparse/ast/ast_directive.hpp>
#include <bcparse/ast/ast_label_decl.hpp>
#include <bcparse/ast/ast_label.hpp>
#include <bcparse/ast/ast_code_body.hpp>
#include <bcparse/ast/ast_string_literal.hpp>
#include <bcparse/ast/ast_integer_literal.hpp>
#include <bcparse/ast/ast_data_location.hpp>
#include <bcparse/ast/ast_variable.hpp>
#include <bcparse/ast/ast_symbol.hpp>
#include <bcparse/ast/ast_interpolation.hpp>
#include <bcparse/ast/ast_jmp_statement.hpp>
#include <bcparse/ast/ast_cmp_statement.hpp>
#include <bcparse/ast/ast_mov_statement.hpp>
#include <bcparse/ast/ast_push_statement.hpp>
#include <bcparse/ast/ast_pop_statement.hpp>
#include <bcparse/ast/ast_binop_statement.hpp>
#include <bcparse/ast/ast_print_statement.hpp>

#include <common/my_assert.hpp>

#include <string>
#include <map>

namespace bcparse {
  Parser::Parser(AstIterator *astIterator,
    TokenStream *tokenStream,
    CompilationUnit *compilationUnit,
    bool variableMode)
    : m_astIterator(astIterator),
      m_tokenStream(tokenStream),
      m_compilationUnit(compilationUnit),
      m_variableMode(variableMode) {
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

    // first pass; hoist macro definitions
    std::vector<Pointer<AstStatement>> hoisted;
    std::vector<Pointer<AstStatement>> otherStmts;

    while (m_tokenStream->hasNext()) {
      if (auto stmt = parseStatement()) {
        if (stmt->isHoisted()) {
          hoisted.push_back(stmt);
        } else {
          otherStmts.push_back(stmt);
        }
      } else {
        break;
      }
    }

    for (const auto &stmt : hoisted) {
      m_astIterator->push(stmt);
    }

    for (const auto &stmt : otherStmts) {
      m_astIterator->push(stmt);
    }
  }

  Pointer<AstStatement> Parser::parseStatement() {
    const SourceLocation location = currentLocation();
    Pointer<AstStatement> res;

    skipStatementTerminators();

    if (!m_tokenStream->hasNext()) {
      return nullptr;
    }

    if (match(Token::TK_DIRECTIVE)) {
      res = parseDirective();
    } else if (match(Token::TK_LABEL)) {
      res = parseLabel();
    } else if (match(Token::TK_IDENT)) {
      res = parseCommand();
    } else {
      res = parseExpression();

      // if (!res) {
      //   m_compilationUnit->getErrorList().addError(CompilerError(
      //     LEVEL_ERROR,
      //     Msg_illegal_expression,
      //     location
      //   ));
      // }
    }

    if (res != nullptr && m_tokenStream->hasNext()) {
      expectEndOfStmt();
    }

    return res;
  }

  Pointer<AstExpression> Parser::parseExpression() {
    if (auto term = parseTerm()) {
      return term;
    }

    return nullptr;
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

    if (match(Token::TK_IDENT)) {
      // diff between symbols / variables --
      // variables check whether the identifier is declared,
      // whereas symbols are used to pass around without any need to be declared
      // an identifier is bound to a value, a symbol is just a complile-time name.
      if (m_variableMode) {
        expr = parseVariable();
      } else {
        expr = parseSymbol();
      }
      // expr = parseVariable();
    } else if (match(Token::TK_OPEN_PARENTH)) {
      // expr = parseParentheses();
    } else if (match(Token::TK_OPEN_BRACKET)) {
      // expr = parseArrayExpression();
    } else if (match(Token::TK_INTEGER)) {
      expr = parseIntegerLiteral();
    } else if (match(Token::TK_FLOAT)) {
      // expr = parseFloatLiteral();
    } else if (match(Token::TK_STRING)) {
      expr = parseStringLiteral();
    } else if (match(Token::TK_INTERPOLATION)) {
      expr = parseInterpolation();
    } else if (match(Token::TK_DATA_LOC)) {
      expr = parseDataLocation();
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

  Pointer<AstVariable> Parser::parseVariable() {
    if (Token token = expect(Token::TK_IDENT, true)) {
      return Pointer<AstVariable>(new AstVariable(
        token.getValue(),
        token.getLocation()
      ));
    }

    return nullptr;
  }

  Pointer<AstSymbol> Parser::parseSymbol() {
    if (Token token = expect(Token::TK_IDENT, true)) {
      return Pointer<AstSymbol>(new AstSymbol(
        token.getValue(),
        token.getLocation()
      ));
    }

    return nullptr;
  }

  Pointer<AstIntegerLiteral> Parser::parseIntegerLiteral() {
    if (Token token = expect(Token::TK_INTEGER, true)) {
      std::istringstream ss(token.getValue());
      int64_t value;
      ss >> value;

      return Pointer<AstIntegerLiteral>(new AstIntegerLiteral(
        value,
        token.getLocation()
      ));
    }

    return nullptr;
  }

  Pointer<AstStringLiteral> Parser::parseStringLiteral() {
    if (Token token = expect(Token::TK_STRING, true)) {
      return Pointer<AstStringLiteral>(new AstStringLiteral(
        token.getValue(),
        token.getLocation()
      ));
    }

    return nullptr;
  }

  Pointer<AstDirective> Parser::parseDirective() {
    if (Token token = expect(Token::TK_DIRECTIVE, true)) {
      std::vector<Pointer<AstExpression>> arguments;
      std::vector<Token> tokens;

      // TODO: parseArguments
      while (m_tokenStream->hasNext() &&
        !match(Token::TK_NEWLINE) &&
        !match(Token::TK_OPEN_BRACE)) {
        if (auto expr = parseTerm()) {
          arguments.emplace_back(expr);
        } else {
          break;
        }
      }

      if (match(Token::TK_OPEN_BRACE, true)) {
        int parenCounter = 1;

        while (m_tokenStream->hasNext()) {
          if (match(Token::TK_OPEN_BRACE)) {
            ++parenCounter;
          } else if (match(Token::TK_CLOSE_BRACE)) {
            --parenCounter;

            if (parenCounter == 0) {
              m_tokenStream->next();
              break;
            }
          }

          tokens.push_back(m_tokenStream->peek());

          m_tokenStream->next();
        }
      }

      return Pointer<AstDirective>(new AstDirective(
        token.getValue(),
        arguments,
        tokens,
        token.getLocation()
      ));
    }

    return nullptr;
  }

  Pointer<AstLabelDecl> Parser::parseLabel() {
    if (Token token = expect(Token::TK_LABEL, true)) {
      // add forward declaration for labels
      Pointer<AstLabel> astLabel(new AstLabel(
        token.getValue(),
        nullptr,
        token.getLocation()
      ));

      m_compilationUnit->getBoundGlobals().set(token.getValue(), astLabel);

      return Pointer<AstLabelDecl>(new AstLabelDecl(
        token.getValue(),
        astLabel,
        token.getLocation()
      ));
    }

    return nullptr;
  }

  Pointer<AstStatement> Parser::parseCommand() {
    if (Token token = expect(Token::TK_IDENT, true)) {
      const std::map<std::string, AstJmpStatement::JumpMode> jumpModeStrings = {
        { "jmp", AstJmpStatement::JumpMode::None },
        { "je", AstJmpStatement::JumpMode::JumpIfEqual },
        { "jne", AstJmpStatement::JumpMode::JumpIfNotEqual },
        { "jg", AstJmpStatement::JumpMode::JumpIfGreater },
        { "jge", AstJmpStatement::JumpMode::JumpIfGreaterOrEqual },
      };

      if (jumpModeStrings.find(token.getValue()) != jumpModeStrings.end()) {
        // m_tokenStream->next();

        auto expr = parseExpression();

        if (!expr) {
          return nullptr;
        }

        return Pointer<AstJmpStatement>(new AstJmpStatement(
          expr,
          jumpModeStrings.find(token.getValue())->second,
          token.getLocation()
        ));
      } else if (token.getValue() == "cmp") {
        // m_tokenStream->next();

        auto left = parseExpression();

        if (!left) {
          return nullptr;
        }

        auto right = parseExpression();

        if (!right) {
          return nullptr;
        }

        return Pointer<AstCmpStatement>(new AstCmpStatement(
          left,
          right,
          token.getLocation()
        ));
      } else if (token.getValue() == "mov") {
        auto left = parseExpression();

        if (!left) {
          return nullptr;
        }

        auto right = parseExpression();

        if (!right) {
          return nullptr;
        }

        return Pointer<AstMovStatement>(new AstMovStatement(
          left,
          right,
          token.getLocation()
        ));
      } else if (token.getValue() == "push") {
        auto arg = parseExpression();

        if (!arg) {
          return nullptr;
        }

        return Pointer<AstPushStatement>(new AstPushStatement(
          arg,
          token.getLocation()
        ));
      } else if (token.getValue() == "pop") {
        size_t num = 1; // 'pop' by default pops 1 value from stack

        if (match(Token::TK_INTEGER)) {
          // TODO: parse unsigned
          auto arg = parseIntegerLiteral();

          if (!arg) {
            return nullptr;
          }

          num = arg->getValue();
        }

        return Pointer<AstPopStatement>(new AstPopStatement(
          num,
          token.getLocation()
        ));
      } else if (std::find(AstBinOpStatement::binaryOperations.begin(), AstBinOpStatement::binaryOperations.end(), token.getValue()) != AstBinOpStatement::binaryOperations.end()) {
        auto left = parseExpression();

        if (!left) {
          return nullptr;
        }

        auto right = parseExpression();

        if (!right) {
          return nullptr;
        }

        return Pointer<AstBinOpStatement>(new AstBinOpStatement(
          token.getValue(),
          left,
          right,
          token.getLocation()
        ));
      } else if (token.getValue() == "print") {
        std::vector<Pointer<AstExpression>> arguments;

        while (m_tokenStream->hasNext() &&
          !match(Token::TK_NEWLINE) &&
          !match(Token::TK_OPEN_BRACE)) {
          if (auto expr = parseTerm()) {
            arguments.emplace_back(expr);
          } else {
            break;
          }
        }

        return Pointer<AstPrintStatement>(new AstPrintStatement(
          arguments,
          token.getLocation()
        ));
      } else if (m_variableMode) {
        m_tokenStream->rewind();

        return parseVariable();
      } // .. more

      m_compilationUnit->getErrorList().addError(CompilerError(
        LEVEL_ERROR,
        Msg_unknown_opcode,
        token.getLocation(),
        token.getValue()
      ));
    }

    return nullptr;
  }

  Pointer<AstExpression> Parser::parseInterpolation() {
    Token token = expect(Token::TK_INTERPOLATION, true);
    if (token.empty()) return nullptr;

    // @TODO evaluate in-place and return the transformed result.
    // @macro directives build their own lexers+parsers with variables that are needed in place.
    // eventually, use reverse polish notation to evaluate, allowing simple operations
    std::string body = token.getValue();

    SourceFile sourceFile(token.getLocation().getFileName(), body.size());
    std::memcpy(sourceFile.getBuffer(), body.data(), body.size());

    SourceStream sourceStream(&sourceFile);
    TokenStream tokenStream(TokenStreamInfo { token.getLocation().getFileName() });

    CompilationUnit tmp(m_compilationUnit->getDataStorage());

    Lexer lexer(sourceStream, &tokenStream, &tmp);

    // add line & column offsets
    lexer.getSourceLocation() += token.getLocation();
    lexer.getSourceLocation().getColumn() += 2 /* for '#{' */;

    lexer.analyze();

    return Pointer<AstInterpolation>(new AstInterpolation(
      tokenStream.getTokens(),
      token.getLocation()
    ));
  }

  Pointer<AstDataLocation> Parser::parseDataLocation() {

    if (Token token = expect(Token::TK_DATA_LOC, true)) {
      std::stringstream ident;
      Pointer<AstIntegerLiteral> offset;

      char ch;
      const std::string tokenValue = token.getValue();

      for (size_t i = 0; i < tokenValue.length(); i++) {
        ch = tokenValue[i];

        if (ch == '[') {
          std::stringstream numVal;

          for (size_t j = i + 1; j < tokenValue.length(); j++) {
            ch = tokenValue[j];

            if (ch == ']') {
              break;
            }

            numVal << ch;
          }

          int64_t num;
          numVal >> num;

          offset.reset(new AstIntegerLiteral(
            num,
            token.getLocation()
          ));

          break;
        }

        ident << ch;
      }

      return Pointer<AstDataLocation>(new AstDataLocation(
        ident.str(),
        offset,
        token.getLocation()
      ));
    }

    m_compilationUnit->getErrorList().addError(CompilerError(
      LEVEL_ERROR,
      Msg_custom_error,
      currentLocation(),
      "Expected a data location"
    ));

    return nullptr;

    // if (match(Token::TK_REG)) {
    //   return parseRegister();
    // }

    // if (match(Token::TK_LOCAL)) {
    //   return parseLocal();
    // }



    // return nullptr;
  }
}
