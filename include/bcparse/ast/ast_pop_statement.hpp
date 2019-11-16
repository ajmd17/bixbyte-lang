#pragma once

#include <string>
#include <memory>

#include <bcparse/ast/ast_statement.hpp>

template <typename T>
using Pointer = std::shared_ptr<T>;

namespace bcparse {
  class AstPopStatement : public AstStatement {
  public:
    AstPopStatement(size_t amt,
      const SourceLocation &location);
    virtual ~AstPopStatement() = default;

    virtual void visit(AstVisitor *visitor, Module *mod) override;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) override;
    virtual void optimize(AstVisitor *visitor, Module *mod) override;

    virtual Pointer<AstStatement> clone() const override;

  private:
    size_t m_amt;

    inline Pointer<AstPopStatement> CloneImpl() const {
      return Pointer<AstPopStatement>(new AstPopStatement(
        m_amt,
        m_location
      ));
    }
  };
}
