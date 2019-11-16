#pragma once

#include <bcparse/ast/ast_expression.hpp>

namespace bcparse {
  class AstVariable : public AstExpression {
  public:
    AstVariable(const std::string &name, const SourceLocation &location);
    virtual ~AstVariable() = default;

    const std::string &getName() const { return m_name; }
    const Pointer<AstExpression> &getValue() const { return m_value; }

    virtual void visit(AstVisitor *visitor, Module *mod) override;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) override;
    virtual void optimize(AstVisitor *visitor, Module *mod) override;

    virtual Pointer<AstStatement> clone() const override;
    virtual AstExpression *getValueOf() override;
    virtual Value getRuntimeValue() const override;
    virtual std::string toString() const override;

  private:
    std::string m_name;

    // set during walk
    Pointer<AstExpression> m_value;

    inline Pointer<AstVariable> CloneImpl() const {
      return Pointer<AstVariable>(new AstVariable(
        m_name,
        m_location
      ));
    }
  };
}
