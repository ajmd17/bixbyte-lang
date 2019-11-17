#pragma once

#include <bcparse/ast/ast_directive.hpp>

namespace bcparse {
  class AstIncludeDirective : public AstDirectiveImpl {
    friend class AstDirective;
  protected:
    AstIncludeDirective(const std::vector<Pointer<AstExpression>> &arguments,
      const std::vector<Token> &tokens,
      const SourceLocation &location);
    virtual ~AstIncludeDirective() override;

    virtual void visit(AstVisitor *visitor, Module *mod) override;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) override;
    virtual void optimize(AstVisitor *visitor, Module *mod) override;

  private:
    AstIterator *m_iterator;
    CompilationUnit *m_compilationUnit;
  };
}
