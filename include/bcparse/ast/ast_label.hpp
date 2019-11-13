#pragma once

#include <bcparse/ast/ast_expression.hpp>

#include <bcparse/emit/obj_loc.hpp>

namespace bcparse {
  class AstLabel : public AstExpression {
  public:
    AstLabel(const std::string &name, const SourceLocation &location);
    virtual ~AstLabel() = default;

    const std::string &getName() const { return m_name; }

    virtual void visit(AstVisitor *visitor, Module *mod) override;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) override;
    virtual void optimize(AstVisitor *visitor, Module *mod) override;

    virtual Pointer<AstStatement> clone() const override;
    virtual std::string toString() const override;

  private:
    std::string m_name;

    inline Pointer<AstLabel> CloneImpl() const {
      return Pointer<AstLabel>(new AstLabel(
        m_name,
        m_location
      ));
    }
  };
}
