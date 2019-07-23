#pragma once

#include <bcparse/token.hpp>
#include <bcparse/source_stream.hpp>
#include <bcparse/compilation_unit.hpp>

namespace bcparse {
  class Lexer {
  public:
    Lexer(const SourceStream &sourceStream,
      TokenStream *tokenStream,
      CompilationUnit *compilationUnit);

    void analyze();

  private:
    bool skipWhitespace();
    bool hasNext();
    Token nextToken();
    char readEscapeCode();
    Token readStringLiteral();
    Token readNumberLiteral();
    Token readHexNumberLiteral();
    Token readComment();
    Token readIdentifier();

    SourceStream m_sourceStream;
    SourceLocation m_sourceLocation;
    TokenStream *m_tokenStream;
    CompilationUnit *m_compilationUnit;

  };
}
