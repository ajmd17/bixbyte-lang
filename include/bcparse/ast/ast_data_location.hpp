#pragma once

#include <bcparse/ast/ast_expression.hpp>
#include <bcparse/ast/ast_integer_literal.hpp>

#include <bcparse/emit/obj_loc.hpp>

namespace bcparse {
  class AstDataLocation : public AstExpression {
  public:
    AstDataLocation(const std::string &ident,
      const Pointer<AstIntegerLiteral> &offset,
      const SourceLocation &location);
    virtual ~AstDataLocation() = default;

    inline const std::string &getIdent() const { return m_ident; }

    virtual void visit(AstVisitor *visitor, Module *mod) override;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) override;
    virtual void optimize(AstVisitor *visitor, Module *mod) override;

    virtual Pointer<AstStatement> clone() const override;

  private:
    std::string m_ident;
    Pointer<AstIntegerLiteral> m_offset;

    // set during walk
    int m_storagePath;

    inline Pointer<AstDataLocation> CloneImpl() const {
      return Pointer<AstDataLocation>(new AstDataLocation(
        m_ident,
        cloneAstNode(m_offset),
        m_location
      ));
    }
  };
}
