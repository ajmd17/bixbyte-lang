#pragma once

#include <bcparse/ast/ast_code_body.hpp>

namespace bcparse {
  class AstInterpolation : public AstCodeBody {
  public:
    AstInterpolation(const std::vector<Token> &tokens,
      const SourceLocation &location);
    virtual ~AstInterpolation() = default;

    virtual void visit(AstVisitor *visitor, Module *mod) override;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) override;

    virtual Pointer<AstStatement> clone() const override;
    virtual AstExpression *getValueOf() override;
    virtual AstExpression *getDeepValueOf() override;
    virtual Value getRuntimeValue() const override;
    virtual std::string toString() const override;

  private:
    inline Pointer<AstInterpolation> CloneImpl() const {
      return Pointer<AstInterpolation>(new AstInterpolation(
        m_tokens,
        m_location
      ));
    }
  };
}
