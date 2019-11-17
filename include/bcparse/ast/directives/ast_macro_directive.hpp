#pragma once

#include <bcparse/ast/ast_directive.hpp>

namespace bcparse {
  class AstMacroDirective : public AstDirectiveImpl {
    friend class AstDirective;
  protected:
    AstMacroDirective(const std::vector<Pointer<AstExpression>> &arguments,
      const std::vector<Token> &tokens,
      const SourceLocation &location);
    virtual ~AstMacroDirective() override;

    virtual void visit(AstVisitor *visitor, Module *mod) override;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) override;
    virtual void optimize(AstVisitor *visitor, Module *mod) override;
  };
}
