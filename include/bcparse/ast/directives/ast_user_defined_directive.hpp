#pragma once

#include <bcparse/ast/ast_directive.hpp>

namespace bcparse {
  class AstUserDefinedDirective : public AstDirectiveImpl {
    friend class AstDirective;
  protected:
    AstUserDefinedDirective(const std::string &name,
      const std::vector<Pointer<AstExpression>> &arguments,
      const std::vector<Token> &tokens,
      const SourceLocation &location);
    virtual ~AstUserDefinedDirective() override;

    virtual void visit(AstVisitor *visitor, Module *mod) override;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) override;
    virtual void optimize(AstVisitor *visitor, Module *mod) override;

    std::string m_name;
    AstIterator *m_iterator;
    CompilationUnit *m_compilationUnit;

  private:
    std::vector<Token> handleInterpolation(const Token &token);
  };
}
