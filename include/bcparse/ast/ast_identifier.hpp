#pragma once

#include <bcparse/ast/ast_expression.hpp>

namespace bcparse {
  class AstIdentifier : public AstExpression {
  public:
    AstIdentifier(const std::string &name, const SourceLocation &location);
    virtual ~AstIdentifier() = default;

    const std::string &getName() const { return m_name; }
    const Pointer<AstExpression> &getValue() const { return m_value; }

    virtual void visit(AstVisitor *visitor, Module *mod) override;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) override;
    virtual void optimize(AstVisitor *visitor, Module *mod) override;

    virtual Pointer<AstStatement> clone() const override;

  private:
    std::string m_name;

    // set during walk
    Pointer<AstExpression> m_value;

    inline Pointer<AstIdentifier> CloneImpl() const {
      return Pointer<AstIdentifier>(new AstIdentifier(
        m_name,
        m_location
      ));
    }
  };
}
