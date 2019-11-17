#pragma once

#include <string>
#include <vector>
#include <memory>

#include <bcparse/ast/ast_statement.hpp>
#include <bcparse/ast/ast_expression.hpp>

template <typename T>
using Pointer = std::shared_ptr<T>;

namespace bcparse {
  class AstPrintStatement : public AstStatement {
  public:
    AstPrintStatement(std::vector<Pointer<AstExpression>> args,
      const SourceLocation &location);
    virtual ~AstPrintStatement() = default;

    virtual void visit(AstVisitor *visitor, Module *mod) override;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) override;
    virtual void optimize(AstVisitor *visitor, Module *mod) override;

    virtual Pointer<AstStatement> clone() const override;

  private:
    std::vector<Pointer<AstExpression>> m_args;

    inline Pointer<AstPrintStatement> CloneImpl() const {
      return Pointer<AstPrintStatement>(new AstPrintStatement(
        cloneAllAstNodes(m_args),
        m_location
      ));
    }
  };
}
