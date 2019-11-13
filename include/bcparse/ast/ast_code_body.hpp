#pragma once

#include <bcparse/ast/ast_string_literal.hpp>

namespace bcparse {
  class AstIterator;
  class CompilationUnit;

  class AstCodeBody : public AstStringLiteral {
  public:
    AstCodeBody(const std::string &value, const SourceLocation &location);
    virtual ~AstCodeBody() override;

    virtual void visit(AstVisitor *visitor, Module *mod) override;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) override;

  private:
    AstIterator *m_iterator;
    CompilationUnit *m_compilationUnit;
  };
}
