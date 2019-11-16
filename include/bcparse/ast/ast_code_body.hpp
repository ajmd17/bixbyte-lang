#pragma once

#include <vector>

#include <bcparse/ast/ast_expression.hpp>

#include <bcparse/token.hpp>

namespace bcparse {
  class AstIterator;
  class CompilationUnit;

  class AstCodeBody : public AstExpression {
  public:
    AstCodeBody(const std::vector<Token> &tokens,
      const SourceLocation &location);
    virtual ~AstCodeBody() override;

    virtual void visit(AstVisitor *visitor, Module *mod) override;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) override;
    virtual void optimize(AstVisitor *visitor, Module *mod) override;

    virtual Pointer<AstStatement> clone() const override;
    virtual std::string toString() const override;

  protected:
    AstCodeBody(const std::vector<Token> &tokens,
      const SourceLocation &location,
      bool variableMode);

    std::vector<Token> m_tokens;

    AstIterator *m_iterator;
    CompilationUnit *m_compilationUnit;
    bool m_variableMode;

  private:
    inline Pointer<AstCodeBody> CloneImpl() const {
      return Pointer<AstCodeBody>(new AstCodeBody(
        m_tokens,
        m_location
      ));
    }
  };
}
