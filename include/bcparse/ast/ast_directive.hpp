#pragma once

#include <string>
#include <vector>
#include <memory>

#include <bcparse/ast/ast_statement.hpp>
#include <bcparse/ast/ast_expression.hpp>

#include <bcparse/token.hpp>

template <typename T>
using Pointer = std::shared_ptr<T>;

namespace bcparse {
  class AstIterator;
  class AstCodeBody;
  class CompilationUnit;

  class AstDirectiveImpl {
    friend class AstDirective;
  protected:
    AstDirectiveImpl(const std::vector<Pointer<AstExpression>> &arguments,
      const std::vector<Token> &tokens,
      const SourceLocation &location);
    AstDirectiveImpl(const AstDirectiveImpl &other) = delete;
    virtual ~AstDirectiveImpl() = default;

    virtual void visit(AstVisitor *visitor, Module *mod) = 0;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) = 0;
    virtual void optimize(AstVisitor *visitor, Module *mod) = 0;

    void visitArguments(AstVisitor *visitor, Module *mod);

    std::vector<Pointer<AstExpression>> m_arguments;
    std::vector<Token> m_tokens;
    SourceLocation m_location;
  };

  class AstMacroDirective : public AstDirectiveImpl {
    friend class AstDirective;
  protected:
    AstMacroDirective(const std::vector<Pointer<AstExpression>> &arguments,
      const std::vector<Token> &tokens,
      const SourceLocation &location);
    virtual ~AstMacroDirective() override;

    virtual void visit(AstVisitor *visitor, Module *mod) override;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) override;
    virtual void optimize(AstVisitor *visitor, Module *mod) override;
  };

  class AstSetDirective : public AstDirectiveImpl {
    friend class AstDirective;
  protected:
    AstSetDirective(const std::vector<Pointer<AstExpression>> &arguments,
      const std::vector<Token> &tokens,
      const SourceLocation &location);
    virtual ~AstSetDirective() override;

    virtual void visit(AstVisitor *visitor, Module *mod) override;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) override;
    virtual void optimize(AstVisitor *visitor, Module *mod) override;
  };

  class AstGetDirective : public AstDirectiveImpl {
    friend class AstDirective;
  protected:
    AstGetDirective(const std::vector<Pointer<AstExpression>> &arguments,
      const std::vector<Token> &tokens,
      const SourceLocation &location);
    virtual ~AstGetDirective() override;

    virtual void visit(AstVisitor *visitor, Module *mod) override;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) override;
    virtual void optimize(AstVisitor *visitor, Module *mod) override;

  private:
    Pointer<AstExpression> m_value;
  };

  class AstYieldDirective : public AstDirectiveImpl {
    friend class AstDirective;
  protected:
    AstYieldDirective(const std::vector<Pointer<AstExpression>> &arguments,
      const std::vector<Token> &tokens,
      const SourceLocation &location);
    virtual ~AstYieldDirective() override;

    virtual void visit(AstVisitor *visitor, Module *mod) override;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) override;
    virtual void optimize(AstVisitor *visitor, Module *mod) override;

  private:
    Pointer<AstCodeBody> m_codeBody;
  };

  class AstDebugDirective : public AstDirectiveImpl {
    friend class AstDirective;
  protected:
    AstDebugDirective(const std::vector<Pointer<AstExpression>> &arguments,
      const std::vector<Token> &tokens,
      const SourceLocation &location);
    virtual ~AstDebugDirective() override;

    virtual void visit(AstVisitor *visitor, Module *mod) override;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) override;
    virtual void optimize(AstVisitor *visitor, Module *mod) override;

  private:
    std::string nodeToString(AstVisitor *visitor, AstExpression *node);
  };

  class AstUserDefinedDirective : public AstDirectiveImpl {
    friend class AstDirective;
  protected:
    AstUserDefinedDirective(const std::string &name,
      const std::vector<Pointer<AstExpression>> &arguments,
      const std::vector<Token> &tokens,
      const SourceLocation &location);
    virtual ~AstUserDefinedDirective() override;

    virtual void visit(AstVisitor *visitor, Module *mod) override;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) override;
    virtual void optimize(AstVisitor *visitor, Module *mod) override;

    std::string m_name;
    AstIterator *m_iterator;
    CompilationUnit *m_compilationUnit;

  private:
    std::vector<Token> handleInterpolation(const Token &token);

  };

  class AstDirective : public AstStatement {
  public:
    AstDirective(const std::string &name,
      const std::vector<Pointer<AstExpression>> &arguments,
      const std::vector<Token> &tokens,
      const SourceLocation &location);
    virtual ~AstDirective() override;

    virtual void visit(AstVisitor *visitor, Module *mod) override;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) override;
    virtual void optimize(AstVisitor *visitor, Module *mod) override;

    virtual Pointer<AstStatement> clone() const override;
    virtual bool isHoisted() const override;

  private:
    std::string m_name;
    std::vector<Pointer<AstExpression>> m_arguments;
    //std::string m_body;
    std::vector<Token> m_tokens;

    AstDirectiveImpl *m_impl;

    inline Pointer<AstDirective> CloneImpl() const {
      return Pointer<AstDirective>(new AstDirective(
        m_name,
        m_arguments,
        m_tokens,
        m_location
      ));
    }
  };
}
