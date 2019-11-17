#pragma once

#include <bcparse/ast_iterator.hpp>
#include <bcparse/token_stream.hpp>
#include <bcparse/token.hpp>
#include <bcparse/compilation_unit.hpp>

template <typename T>
using Pointer = std::shared_ptr<T>;

namespace bcparse {
  class AstExpression;
  class AstDirective;
  class AstStringLiteral;
  class AstIntegerLiteral;
  class AstVariable;
  class AstSymbol;
  class AstInterpolation;
  class AstLabelDecl;
  class AstDataLocation;

  class Parser {
  public:
    Parser(AstIterator *astIterator,
      TokenStream *tokenStream,
      CompilationUnit *compilationUnit,
      bool variableMode = false);

    Parser(const Parser &other) = delete;

    void parse();

  private:
    Token match(Token::TokenClass tokenClass, bool read = false);
    Token matchAhead(Token::TokenClass tokenClass, int n);
    Token expect(Token::TokenClass tokenClass, bool read = false);
    bool expectEndOfStmt();
    SourceLocation currentLocation() const;
    void skipStatementTerminators();

    Pointer<AstStatement> parseStatement();
    Pointer<AstStatement> parseCommand();
    Pointer<AstExpression> parseExpression();
    Pointer<AstExpression> parseTerm();
    Pointer<AstStringLiteral> parseStringLiteral();
    Pointer<AstIntegerLiteral> parseIntegerLiteral();
    Pointer<AstVariable> parseVariable();
    Pointer<AstSymbol> parseSymbol();
    Pointer<AstDirective> parseDirective();
    Pointer<AstLabelDecl> parseLabel();
    Pointer<AstExpression> parseInterpolation();
    Pointer<AstDataLocation> parseDataLocation();

    AstIterator *m_astIterator;
    TokenStream *m_tokenStream;
    CompilationUnit *m_compilationUnit;
    bool m_variableMode;
  };
}
