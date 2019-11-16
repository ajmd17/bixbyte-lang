#pragma once

#include <string>
#include <vector>
#include <memory>

#include <bcparse/ast/ast_statement.hpp>
#include <bcparse/ast/ast_expression.hpp>

template <typename T>
using Pointer = std::shared_ptr<T>;

namespace bcparse {
  class AstPushStatement : public AstStatement {
  public:
    AstPushStatement(Pointer<AstExpression> arg, // could be an expression or a data location
      const SourceLocation &location);
    virtual ~AstPushStatement() = default;

    virtual void visit(AstVisitor *visitor, Module *mod) override;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) override;
    virtual void optimize(AstVisitor *visitor, Module *mod) override;

    virtual Pointer<AstStatement> clone() const override;

  private:
    Pointer<AstExpression> m_arg;

    inline Pointer<AstPushStatement> CloneImpl() const {
      return Pointer<AstPushStatement>(new AstPushStatement(
        cloneAstNode(m_arg),
        m_location
      ));
    }
  };
}
