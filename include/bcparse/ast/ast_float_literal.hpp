#pragma once

#include <bcparse/ast/ast_expression.hpp>

template <typename T>
using Pointer = std::shared_ptr<T>;

namespace bcparse {
  class AstFloatLiteral : public AstExpression {
  public:
    AstFloatLiteral(double value, const SourceLocation &location);
    virtual ~AstFloatLiteral() = default;

    const double getValue() const { return m_value; }

    virtual void visit(AstVisitor *visitor, Module *mod) override;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) override;
    virtual void optimize(AstVisitor *visitor, Module *mod) override;

    virtual Pointer<AstStatement> clone() const override;
    virtual std::string toString() const override;
    virtual Value getRuntimeValue() const override;

  private:
    double m_value;

    inline Pointer<AstFloatLiteral> CloneImpl() const {
      return Pointer<AstFloatLiteral>(new AstFloatLiteral(
        m_value,
        m_location
      ));
    }
  };
}
