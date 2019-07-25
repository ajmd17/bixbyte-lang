#pragma once

#include <string>
#include <vector>
#include <memory>

#include <bcparse/ast/ast_statement.hpp>
#include <bcparse/ast/ast_expression.hpp>

template <typename T>
using Pointer = std::shared_ptr<T>;

namespace bcparse {
  class AstDirective : public AstStatement {
  public:
    AstDirective(const std::string &name,
      const std::vector<Pointer<AstExpression>> &arguments,
      const std::string &body,
      const SourceLocation &location);

    virtual void visit(AstVisitor *visitor, Module *mod) override;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) override;
    virtual void optimize(AstVisitor *visitor, Module *mod) override;

    virtual Pointer<AstStatement> clone() const override;

  private:
    std::string m_name;
    std::vector<Pointer<AstExpression>> m_arguments;
    std::string m_body;

    inline Pointer<AstDirective> CloneImpl() const {
      return Pointer<AstDirective>(new AstDirective(
        m_name,
        m_arguments,
        m_body,
        m_location
      ));
    }
  };
}
