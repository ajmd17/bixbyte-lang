#pragma once

#include <bcparse/ast/ast_expression.hpp>

namespace bcparse {
  class AstInterpolation : public AstExpression {
  public:
    AstInterpolation(Pointer<AstExpression> value, const SourceLocation &location);
    virtual ~AstInterpolation() = default;

    const Pointer<AstExpression> &getValue() const { return m_value; }

    virtual void visit(AstVisitor *visitor, Module *mod) override;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) override;
    virtual void optimize(AstVisitor *visitor, Module *mod) override;

    virtual Pointer<AstStatement> clone() const override;
    virtual AstExpression *getValueOf() override;

  private:
    Pointer<AstExpression> m_value;

    inline Pointer<AstInterpolation> CloneImpl() const {
      return Pointer<AstInterpolation>(new AstInterpolation(
        cloneAstNode(m_value),
        m_location
      ));
    }
  };
}
