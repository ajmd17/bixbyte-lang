#pragma once

#include <bcparse/ast/ast_expression.hpp>

namespace bcparse {
  class AstIntegerLiteral : public AstExpression {
  public:
    AstIntegerLiteral(int64_t value, const SourceLocation &location);
    virtual ~AstIntegerLiteral() = default;

    const int64_t getValue() const { return m_value; }

    virtual void visit(AstVisitor *visitor, Module *mod) override;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) override;
    virtual void optimize(AstVisitor *visitor, Module *mod) override;

    virtual Pointer<AstStatement> clone() const override;
    virtual std::string toString() const override;

  private:
    int64_t m_value;

    inline Pointer<AstIntegerLiteral> CloneImpl() const {
      return Pointer<AstIntegerLiteral>(new AstIntegerLiteral(
        m_value,
        m_location
      ));
    }
  };
}
