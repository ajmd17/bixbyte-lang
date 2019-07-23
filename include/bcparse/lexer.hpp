#pragma once

#include <bcparse/source_stream.hpp>
#include <bcparse/source_location.hpp>

namespace bcparse {
  class Token;
  class SourceStream;
  class TokenStream;
  class CompilationUnit;

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
    utf::u32char readEscapeCode();
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
