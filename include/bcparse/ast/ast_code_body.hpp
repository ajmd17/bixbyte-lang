#pragma once

#include <bcparse/ast/ast_expression.hpp>

namespace bcparse {
  class AstIterator;
  class CompilationUnit;

  class AstCodeBody : public AstExpression {
  public:
    AstCodeBody(const std::string &value, const SourceLocation &location);
    virtual ~AstCodeBody() override;

    virtual void visit(AstVisitor *visitor, Module *mod) override;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) override;
    virtual void optimize(AstVisitor *visitor, Module *mod) override;

    virtual Pointer<AstStatement> clone() const override;
    virtual std::string toString() const override;

  private:
    std::string m_value;

    AstIterator *m_iterator;
    CompilationUnit *m_compilationUnit;

    inline Pointer<AstCodeBody> CloneImpl() const {
      return Pointer<AstCodeBody>(new AstCodeBody(
        m_value,
        m_location
      ));
    }
  };
}
