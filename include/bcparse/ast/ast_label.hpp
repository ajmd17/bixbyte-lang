#pragma once

#include <bcparse/ast/ast_expression.hpp>
#include <bcparse/ast/ast_data_location.hpp>

namespace bcparse {
  class AstLabel : public AstExpression {
  public:
    AstLabel(const std::string &name,
      const Pointer<AstDataLocation> &dataLocation,
      const SourceLocation &location);
    virtual ~AstLabel() = default;

    const std::string &getName() const { return m_name; }
    Pointer<AstDataLocation> &getDataLocation() { return m_dataLocation; }
    const Pointer<AstDataLocation> &getDataLocation() const { return m_dataLocation; }

    virtual void visit(AstVisitor *visitor, Module *mod) override;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) override;
    virtual void optimize(AstVisitor *visitor, Module *mod) override;

    virtual Pointer<AstStatement> clone() const override;
    virtual AstExpression *getValueOf() override;
    virtual AstExpression *getDeepValueOf() override;
    virtual std::string toString() const override;

  private:
    std::string m_name;
    Pointer<AstDataLocation> m_dataLocation;

    inline Pointer<AstLabel> CloneImpl() const {
      return Pointer<AstLabel>(new AstLabel(
        m_name,
        cloneAstNode(m_dataLocation),
        m_location
      ));
    }
  };
}
