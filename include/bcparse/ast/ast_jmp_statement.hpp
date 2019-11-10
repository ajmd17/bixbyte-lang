#pragma once

#include <string>
#include <vector>
#include <memory>

#include <bcparse/ast/ast_statement.hpp>
#include <bcparse/ast/ast_expression.hpp>

template <typename T>
using Pointer = std::shared_ptr<T>;

namespace bcparse {
  class AstJmpStatement : public AstStatement {
  public:
    enum class JumpMode {
      None = 0,
      JumpIfEqual = 1,
      JumpIfNotEqual = 2,
      JumpIfGreater = 3,
      JumpIfGreaterOrEqual = 4
    };

    AstJmpStatement(Pointer<AstExpression> arg,
      JumpMode jumpMode,
      const SourceLocation &location);
    virtual ~AstJmpStatement() = default;

    inline JumpMode getJumpMode() const { return m_jumpMode; }

    virtual void visit(AstVisitor *visitor, Module *mod) override;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) override;
    virtual void optimize(AstVisitor *visitor, Module *mod) override;

    virtual Pointer<AstStatement> clone() const override;

  private:
    Pointer<AstExpression> m_arg;
    JumpMode m_jumpMode;

    // set during walk
    Pointer<AstExpression> m_pointee;

    inline Pointer<AstJmpStatement> CloneImpl() const {
      return Pointer<AstJmpStatement>(new AstJmpStatement(
        cloneAstNode(m_arg),
        m_jumpMode,
        m_location
      ));
    }
  };
}
