#pragma once

#include <string>
#include <vector>
#include <memory>

#include <bcparse/ast/ast_statement.hpp>
#include <bcparse/ast/ast_expression.hpp>

template <typename T>
using Pointer = std::shared_ptr<T>;

namespace bcparse {
  class AstMovStatement : public AstStatement {
  public:
    AstMovStatement(Pointer<AstExpression> left,
      Pointer<AstExpression> right,
      const SourceLocation &location);
    virtual ~AstMovStatement() = default;

    virtual void visit(AstVisitor *visitor, Module *mod) override;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) override;
    virtual void optimize(AstVisitor *visitor, Module *mod) override;

    virtual Pointer<AstStatement> clone() const override;

  private:
    Pointer<AstExpression> m_left;
    Pointer<AstExpression> m_right;

    inline Pointer<AstMovStatement> CloneImpl() const {
      return Pointer<AstMovStatement>(new AstMovStatement(
        cloneAstNode(m_left),
        cloneAstNode(m_right),
        m_location
      ));
    }
  };
}
