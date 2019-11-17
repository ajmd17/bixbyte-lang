#pragma once

#include <string>
#include <vector>
#include <memory>

#include <bcparse/ast/ast_statement.hpp>
#include <bcparse/ast/ast_expression.hpp>

template <typename T>
using Pointer = std::shared_ptr<T>;

namespace bcparse {
  class AstBinOpStatement : public AstStatement {
  public:
    static const std::vector<std::string> binaryOperations;

    AstBinOpStatement(const std::string &opName,
      Pointer<AstExpression> left,
      Pointer<AstExpression> right,
      const SourceLocation &location);
    virtual ~AstBinOpStatement() = default;

    virtual void visit(AstVisitor *visitor, Module *mod) override;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) override;
    virtual void optimize(AstVisitor *visitor, Module *mod) override;

    virtual Pointer<AstStatement> clone() const override;

  private:
    std::string m_opName;
    Pointer<AstExpression> m_left;
    Pointer<AstExpression> m_right;

    inline Pointer<AstBinOpStatement> CloneImpl() const {
      return Pointer<AstBinOpStatement>(new AstBinOpStatement(
        m_opName,
        cloneAstNode(m_left),
        cloneAstNode(m_right),
        m_location
      ));
    }
  };
}
