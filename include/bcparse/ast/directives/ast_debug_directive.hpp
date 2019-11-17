#pragma once

#include <bcparse/ast/ast_directive.hpp>

namespace bcparse {
  class AstDebugDirective : public AstDirectiveImpl {
    friend class AstDirective;
  protected:
    AstDebugDirective(const std::vector<Pointer<AstExpression>> &arguments,
      const std::vector<Token> &tokens,
      const SourceLocation &location);
    virtual ~AstDebugDirective() override;

    virtual void visit(AstVisitor *visitor, Module *mod) override;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) override;
    virtual void optimize(AstVisitor *visitor, Module *mod) override;
  };
}
