#pragma once

#include <bcparse/ast/ast_statement.hpp>
#include <bcparse/ast/ast_label.hpp>

namespace bcparse {
  class AstLabelDecl : public AstStatement {
  public:
    AstLabelDecl(const std::string &name,
      Pointer<AstLabel> astLabel,
      const SourceLocation &location);
    virtual ~AstLabelDecl() = default;

    const std::string &getName() const { return m_name; }

    virtual void visit(AstVisitor *visitor, Module *mod) override;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) override;
    virtual void optimize(AstVisitor *visitor, Module *mod) override;

    virtual Pointer<AstStatement> clone() const override;

  private:
    std::string m_name;
    Pointer<AstLabel> m_astLabel;

    inline Pointer<AstLabelDecl> CloneImpl() const {
      return Pointer<AstLabelDecl>(new AstLabelDecl(
        m_name,
        cloneAstNode(m_astLabel),
        m_location
      ));
    }
  };
}
