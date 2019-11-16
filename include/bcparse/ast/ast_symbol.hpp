#pragma once
#if 0
#define AstSymbol AstVariable

#else

#include <bcparse/ast/ast_expression.hpp>

namespace bcparse {
  class AstSymbol : public AstExpression {
  public:
    AstSymbol(const std::string &name, const SourceLocation &location);
    virtual ~AstSymbol() = default;

    const std::string &getName() const { return m_name; }

    virtual void visit(AstVisitor *visitor, Module *mod) override;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) override;
    virtual void optimize(AstVisitor *visitor, Module *mod) override;

    virtual Pointer<AstStatement> clone() const override;

  private:
    std::string m_name;

    inline Pointer<AstSymbol> CloneImpl() const {
      return Pointer<AstSymbol>(new AstSymbol(
        m_name,
        m_location
      ));
    }
  };
}

#endif
