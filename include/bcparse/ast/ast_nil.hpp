#pragma once

#include <bcparse/ast/ast_expression.hpp>

namespace bcparse {
  class AstNil : public AstExpression {
  public:
    AstNil(const SourceLocation &location);
    virtual ~AstNil() = default;

    virtual void visit(AstVisitor *visitor, Module *mod) override;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) override;
    virtual void optimize(AstVisitor *visitor, Module *mod) override;

    virtual Pointer<AstStatement> clone() const override;
    virtual std::string toString() const override;
    virtual Value getRuntimeValue() const override;

  private:
    inline Pointer<AstNil> CloneImpl() const {
      return Pointer<AstNil>(new AstNil(
        m_location
      ));
    }
  };
}
