#include <bcparse/ast/ast_directive.hpp>

#include <bcparse/ast/directives/ast_macro_directive.hpp>
#include <bcparse/ast/directives/ast_var_directive.hpp>
#include <bcparse/ast/directives/ast_set_directive.hpp>
#include <bcparse/ast/directives/ast_debug_directive.hpp>
#include <bcparse/ast/directives/ast_user_defined_directive.hpp>

#include <bcparse/emit/bytecode_chunk.hpp>

#include <bcparse/ast_visitor.hpp>
#include <bcparse/compilation_unit.hpp>
#include <bcparse/bound_variables.hpp>
#include <bcparse/source_file.hpp>

#include <common/my_assert.hpp>

#include <sstream>
#include <iostream>
#include <algorithm>

namespace bcparse {
  AstDirectiveImpl::AstDirectiveImpl(const std::vector<Pointer<AstExpression>> &arguments,
    const std::vector<Token> &tokens,
    const SourceLocation &location)
    : m_arguments(arguments),
      m_tokens(tokens),
      m_location(location) {
  }

  void AstDirectiveImpl::visitArguments(AstVisitor *visitor, Module *mod) {
    for (auto &arg : m_arguments) {
      ASSERT(arg != nullptr);

      arg->visit(visitor, mod);
    }
  }


  AstDirective::AstDirective(const std::string &name,
    const std::vector<Pointer<AstExpression>> &arguments,
    const std::vector<Token> &tokens,
    const SourceLocation &location)
    : AstStatement(location),
      m_name(name),
      m_arguments(arguments),
      m_tokens(tokens),
      m_impl(nullptr) {
  }

  AstDirective::~AstDirective() {
    if (m_impl) {
      delete m_impl;
      m_impl = nullptr;
    }
  }

  void AstDirective::visit(AstVisitor *visitor, Module *mod) {
    ASSERT(m_impl == nullptr);

    if (m_name == "macro") {
      m_impl = new AstMacroDirective(m_arguments, m_tokens, m_location);
    } else if (m_name == "var") {
      m_impl = new AstVarDirective(m_arguments, m_tokens, m_location);
    } else if (m_name == "set") {
      m_impl = new AstSetDirective(m_arguments, m_tokens, m_location);
    } else if (m_name == "debug") {
      m_impl = new AstDebugDirective(m_arguments, m_tokens, m_location);
    } else if (visitor->getCompilationUnit()->getBoundGlobals().lookupMacro(m_name)) {
      m_impl = new AstUserDefinedDirective(m_name, m_arguments, m_tokens, m_location);
    }

    if (m_impl != nullptr) {
      m_impl->visit(visitor, mod);
    } else {
      visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
        LEVEL_ERROR,
        Msg_unknown_directive,
        m_location,
        m_name
      ));
    }
  }

  void AstDirective::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
    if (m_impl == nullptr) return;

    m_impl->build(visitor, mod, out);
  }

  void AstDirective::optimize(AstVisitor *visitor, Module *mod) {
    if (m_impl == nullptr) return;

    m_impl->optimize(visitor, mod);
  }

  Pointer<AstStatement> AstDirective::clone() const {
    return CloneImpl();
  }

  bool AstDirective::isHoisted() const {
    return m_name == "macro";
  }
}
