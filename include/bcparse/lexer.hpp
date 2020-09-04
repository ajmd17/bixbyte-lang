#pragma once

#include <shared/source_location.hpp>

#include <bcparse/source_stream.hpp>
#include <bcparse/token.hpp>

namespace bcparse {
  class SourceStream;
  class TokenStream;
  class CompilationUnit;

  class Lexer {
  public:
    Lexer(const SourceStream &sourceStream,
      TokenStream *tokenStream,
      CompilationUnit *compilationUnit);

    inline SourceLocation &getSourceLocation() { return m_sourceLocation; }
    inline const SourceLocation &getSourceLocation() const { return m_sourceLocation; }

    void analyze();

  private:
    bool expectChar(utf::u32char ch, bool read = false, int *posChange = nullptr);

    bool skipWhitespace();
    bool hasNext();
    Token nextToken();
    utf::u32char readEscapeCode();
    Token readStringLiteral();
    Token readNumberLiteral();
    Token readHexNumberLiteral();
    Token readComment();
    Token readIdentifier();
    Token readDirective();
    Token readDataLocation();
    Token readInterpolation();

    SourceStream m_sourceStream;
    SourceLocation m_sourceLocation;
    TokenStream *m_tokenStream;
    CompilationUnit *m_compilationUnit;

  };
}
