#pragma once

#include <string>
#include <vector>
#include <memory>

#include <bcparse/ast/ast_statement.hpp>
#include <bcparse/ast/ast_expression.hpp>

template <typename T>
using Pointer = std::shared_ptr<T>;

namespace bcparse {
  class AstCallStatement : public AstStatement {
  public:
    AstCallStatement(std::vector<Pointer<AstExpression>> args,
      const SourceLocation &location);
    virtual ~AstCallStatement() = default;

    virtual void visit(AstVisitor *visitor, Module *mod) override;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) override;
    virtual void optimize(AstVisitor *visitor, Module *mod) override;

    virtual Pointer<AstStatement> clone() const override;

  private:
    std::vector<Pointer<AstExpression>> m_args;

    inline Pointer<AstCallStatement> CloneImpl() const {
      return Pointer<AstCallStatement>(new AstCallStatement(
        cloneAllAstNodes(m_args),
        m_location
      ));
    }
  };
}
