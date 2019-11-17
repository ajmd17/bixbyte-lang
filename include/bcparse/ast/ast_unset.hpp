#pragma once

#include <bcparse/ast/ast_expression.hpp>

namespace bcparse {
  class AstUnset : public AstExpression {
  public:
    AstUnset(const SourceLocation &location);
    virtual ~AstUnset() = default;

    virtual void visit(AstVisitor *visitor, Module *mod) override;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) override;
    virtual void optimize(AstVisitor *visitor, Module *mod) override;

    virtual Pointer<AstStatement> clone() const override;
    virtual std::string toString() const override;

  private:
    inline Pointer<AstUnset> CloneImpl() const {
      return Pointer<AstUnset>(new AstUnset(
        m_location
      ));
    }
  };
}
