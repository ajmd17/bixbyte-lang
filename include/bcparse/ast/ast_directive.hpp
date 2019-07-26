#pragma once

#include <string>
#include <vector>
#include <memory>

#include <bcparse/ast/ast_statement.hpp>
#include <bcparse/ast/ast_expression.hpp>

template <typename T>
using Pointer = std::shared_ptr<T>;

namespace bcparse {
  class AstIterator;
  class CompilationUnit;

  class AstDirectiveImpl {
    friend class AstDirective;
  protected:
    AstDirectiveImpl(const std::vector<Pointer<AstExpression>> &arguments,
      const std::string &body,
      const SourceLocation &location);
    AstDirectiveImpl(const AstDirectiveImpl &other) = delete;
    virtual ~AstDirectiveImpl() = default;
    
    virtual void visit(AstVisitor *visitor, Module *mod) = 0;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) = 0;
    virtual void optimize(AstVisitor *visitor, Module *mod) = 0;

    std::vector<Pointer<AstExpression>> m_arguments;
    std::string m_body;
    SourceLocation m_location;
  };

  class AstMacroDirective : public AstDirectiveImpl {
    friend class AstDirective;
  protected:
    AstMacroDirective(const std::vector<Pointer<AstExpression>> &arguments,
      const std::string &body,
      const SourceLocation &location);
    virtual ~AstMacroDirective() override;

    virtual void visit(AstVisitor *visitor, Module *mod) override;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) override;
    virtual void optimize(AstVisitor *visitor, Module *mod) override;
  };

  class AstUserDefinedDirective : public AstDirectiveImpl {
    friend class AstDirective;
  protected:
    AstUserDefinedDirective(const std::string &name,
      const std::vector<Pointer<AstExpression>> &arguments,
      const std::string &body,
      const SourceLocation &location);
    virtual ~AstUserDefinedDirective() override;

    virtual void visit(AstVisitor *visitor, Module *mod) override;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) override;
    virtual void optimize(AstVisitor *visitor, Module *mod) override;

    std::string m_name;
    AstIterator *m_iterator;
    CompilationUnit *m_compilationUnit;
  };

  class AstDirective : public AstStatement {
  public:
    AstDirective(const std::string &name,
      const std::vector<Pointer<AstExpression>> &arguments,
      const std::string &body,
      const SourceLocation &location);
    virtual ~AstDirective() override;

    virtual void visit(AstVisitor *visitor, Module *mod) override;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) override;
    virtual void optimize(AstVisitor *visitor, Module *mod) override;

    virtual Pointer<AstStatement> clone() const override;

  private:
    std::string m_name;
    std::vector<Pointer<AstExpression>> m_arguments;
    std::string m_body;

    AstDirectiveImpl *m_impl;

    inline Pointer<AstDirective> CloneImpl() const {
      return Pointer<AstDirective>(new AstDirective(
        m_name,
        m_arguments,
        m_body,
        m_location
      ));
    }
  };
}
