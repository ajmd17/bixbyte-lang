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
  class AstIdentifier;
  class AstLabelDecl;

  class Parser {
  public:
    Parser(AstIterator *astIterator,
      TokenStream *tokenStream,
      CompilationUnit *compilationUnit);

    Parser(const Parser &other);

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
    Pointer<AstIdentifier> parseIdentifier();
    Pointer<AstDirective> parseDirective();
    Pointer<AstLabelDecl> parseLabel();
    Pointer<AstExpression> parseInterpolation();

    AstIterator *m_astIterator;
    TokenStream *m_tokenStream;
    CompilationUnit *m_compilationUnit;
  };
}
