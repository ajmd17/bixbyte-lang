#pragma once

#include <bcparse/ast/ast_expression.hpp>

namespace bcparse {
  class AstStringLiteral : public AstExpression {
  public:
    AstStringLiteral(const std::string &value, const SourceLocation &location);
    virtual ~AstStringLiteral() = default;
    
    const std::string &getValue() const { return m_value; }

    virtual void visit(AstVisitor *visitor, Module *mod) override;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) override;
    virtual void optimize(AstVisitor *visitor, Module *mod) override;

    virtual Pointer<AstStatement> clone() const override;

  private:
    std::string m_value;

    inline Pointer<AstStringLiteral> CloneImpl() const {
      return Pointer<AstStringLiteral>(new AstStringLiteral(
        m_value,
        m_location
      ));
    }
  };
}
