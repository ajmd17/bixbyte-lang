#include <bcparse/ast/ast_directive.hpp>
#include <bcparse/ast/ast_string_literal.hpp>
#include <bcparse/ast/ast_integer_literal.hpp>
#include <bcparse/ast/ast_variable.hpp>
#include <bcparse/ast/ast_symbol.hpp>
#include <bcparse/ast/ast_code_body.hpp>

#include <bcparse/emit/bytecode_chunk.hpp>

#include <bcparse/lexer.hpp>
#include <bcparse/parser.hpp>
#include <bcparse/analyzer.hpp>
#include <bcparse/compiler.hpp>
#include <bcparse/ast_iterator.hpp>
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

  AstMacroDirective::AstMacroDirective(const std::vector<Pointer<AstExpression>> &arguments,
    const std::vector<Token> &tokens,
    const SourceLocation &location)
    : AstDirectiveImpl(arguments, tokens, location) {
  }

  AstMacroDirective::~AstMacroDirective() {
  }

  void AstMacroDirective::visit(AstVisitor *visitor, Module *mod) {
    const size_t numErrors = visitor->getCompilationUnit()->getErrorList().m_errors.size();

    AstSymbol *nameArg = nullptr;

    if (m_arguments.size() != 1) {
      visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
        LEVEL_ERROR,
        Msg_custom_error,
        m_location,
        "@macro requires arguments (name)"
      ));
    } else {
      AstExpression *deepValue = nullptr;

      if (auto nameArgExpr = m_arguments[0].get()) {
        nameArgExpr->visit(visitor, mod);

        if ((deepValue = nameArgExpr->getDeepValueOf()) != nullptr) {
          nameArg = dynamic_cast<AstSymbol*>(deepValue);
        }
      }

      if (nameArg == nullptr) {
        if (m_arguments[0] != nullptr && deepValue != nullptr) {
          visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
            LEVEL_ERROR,
            Msg_custom_error,
            m_arguments[0]->getLocation(),
            "@set (key) must be an identifier, got %",
            AstExpression::nodeToString(visitor, deepValue)
          ));
        } else {
          visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
            LEVEL_ERROR,
            Msg_custom_error,
            m_location,
            "@set (key) must be an identifier"
          ));
        }
      } else {
        if (Macro *macro = visitor->getCompilationUnit()->getBoundGlobals().lookupMacro(nameArg->getName())) {
          visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
            LEVEL_ERROR,
            Msg_custom_error,
            m_location,
            std::string("@macro `") + nameArg->getName() + "` already defined"
          ));
        }
      }
    }

    if (m_tokens.empty()) {
      visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
        LEVEL_ERROR,
        Msg_custom_error,
        m_location,
        "@macro missing body"
      ));
    }

    if (visitor->getCompilationUnit()->getErrorList().m_errors.size() > numErrors) return;

    visitor->getCompilationUnit()->getBoundGlobals().defineMacro(nameArg->getName(), m_tokens);
  }

  void AstMacroDirective::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
  }

  void AstMacroDirective::optimize(AstVisitor *visitor, Module *mod) {
  }


  AstGetDirective::AstGetDirective(const std::vector<Pointer<AstExpression>> &arguments,
    const std::vector<Token> &tokens,
    const SourceLocation &location)
    : AstDirectiveImpl(arguments, tokens, location),
      m_value(nullptr) {
  }

  AstGetDirective::~AstGetDirective() {
  }

  void AstGetDirective::visit(AstVisitor *visitor, Module *mod) {
    AstSymbol *nameArg = nullptr;

    if (m_arguments.size() != 1) {
      visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
        LEVEL_ERROR,
        Msg_custom_error,
        m_location,
        "@get requires arguments (key)"
      ));
    } else {
      if (auto nameArgExpr = m_arguments[0].get()) {
        nameArgExpr->visit(visitor, mod);

        if (nameArgExpr->getDeepValueOf() != nullptr) {
          nameArg = dynamic_cast<AstSymbol*>(nameArgExpr->getDeepValueOf());
        }
      }

      if (nameArg == nullptr) {
        visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
          LEVEL_ERROR,
          Msg_custom_error,
          m_location,
          "@get (key) must be an identifier"
        ));
      } else {
        m_value = visitor->getCompilationUnit()->getBoundGlobals().get(nameArg->getName());

        if (m_value != nullptr) {
          m_value->visit(visitor, mod);
        }
      }
    }
  }

  void AstGetDirective::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
    if (m_value != nullptr) {
      m_value->build(visitor, mod, out);
    }
  }

  void AstGetDirective::optimize(AstVisitor *visitor, Module *mod) {
  }


  AstSetDirective::AstSetDirective(const std::vector<Pointer<AstExpression>> &arguments,
    const std::vector<Token> &tokens,
    const SourceLocation &location)
    : AstDirectiveImpl(arguments, tokens, location) {
  }

  AstSetDirective::~AstSetDirective() {
  }

  void AstSetDirective::visit(AstVisitor *visitor, Module *mod) {
    AstSymbol *nameArg = nullptr;
    AstExpression *valueArg = nullptr;

    if (m_arguments.size() != 2) {
      visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
        LEVEL_ERROR,
        Msg_custom_error,
        m_location,
        "@set requires arguments (key, value)"
      ));
    } else {
      AstExpression *deepValue = nullptr;

      if (auto nameArgExpr = m_arguments[0].get()) {
        nameArgExpr->visit(visitor, mod);

        if ((deepValue = nameArgExpr->getDeepValueOf()) != nullptr) {
          nameArg = dynamic_cast<AstSymbol*>(deepValue);
        }
      }

      if (nameArg == nullptr) {
        if (m_arguments[0] != nullptr && deepValue != nullptr) {
          visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
            LEVEL_ERROR,
            Msg_custom_error,
            m_arguments[0]->getLocation(),
            "@set (key) must be an identifier, got %",
            AstExpression::nodeToString(visitor, deepValue)
          ));
        } else {
          visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
            LEVEL_ERROR,
            Msg_custom_error,
            m_location,
            "@set (key) must be an identifier"
          ));
        }
      }

      if ((valueArg = m_arguments[1].get())) {
        valueArg->visit(visitor, mod);

        if (valueArg->getDeepValueOf() != nullptr) {
          valueArg = valueArg->getDeepValueOf();
        }
      }

      if (valueArg == nullptr) {
        visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
          LEVEL_ERROR,
          Msg_custom_error,
          m_location,
          "@set (value) must be an expression"
        ));
      }

      if (nameArg != nullptr && valueArg != nullptr) {
        // if currently in global scope, set the var as a global.
        // if not, bubble up to the scope highest enough to not reach global
        BoundVariables *boundVariables = &visitor->getCompilationUnit()->getBoundGlobals();

        while (boundVariables->getParent() != nullptr) {
          boundVariables = boundVariables->getParent();
        }

        Pointer<AstExpression> clonedExpr = std::dynamic_pointer_cast<AstExpression>(valueArg->clone());

        boundVariables->set(nameArg->getName(), clonedExpr);
      }
    }
  }

  void AstSetDirective::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
  }

  void AstSetDirective::optimize(AstVisitor *visitor, Module *mod) {
  }


  AstYieldDirective::AstYieldDirective(const std::vector<Pointer<AstExpression>> &arguments,
    const std::vector<Token> &tokens,
    const SourceLocation &location)
    : AstDirectiveImpl(arguments, tokens, location) {
  }

  AstYieldDirective::~AstYieldDirective() {
  }

  void AstYieldDirective::visit(AstVisitor *visitor, Module *mod) {
    AstSymbol *nameArg = nullptr;

    if (m_arguments.size() != 1) {
      visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
        LEVEL_ERROR,
        Msg_custom_error,
        m_location,
        "@yield requires arguments (key)"
      ));
    } else {
      if (m_arguments[0] != nullptr && m_arguments[0]->getValueOf() != nullptr) {
        nameArg = dynamic_cast<AstSymbol*>(m_arguments[0]->getValueOf());
      }

      if (nameArg == nullptr) {
        visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
          LEVEL_ERROR,
          Msg_custom_error,
          m_location,
          "@yield (key) must be an identifier"
        ));
      }

      if (nameArg != nullptr) {
        // TODO: define a macro named by whatever name was provided,
        // and make the body of the macro set a variable to contain the yielded body
        std::string macroName = nameArg->getName();
        std::string varName = std::string("__") + macroName + "__yield_body";

        ASSERT_MSG(false, "Not implemented");
        // m_codeBody = Pointer<AstCodeBody>(new AstCodeBody(
        //   std::string("@macro ") + macroName + " {\n"
        //     + std::string("  @set ") + varName + " #{body}\n"
        //     + std::string("}\n")
        //     + std::string("@get ") + varName + "\n",
        //   m_location
        // ));

        // m_codeBody->visit(visitor, mod);
      }
    }
  }

  void AstYieldDirective::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
    ASSERT(m_codeBody != nullptr);

    m_codeBody->build(visitor, mod, out);
  }

  void AstYieldDirective::optimize(AstVisitor *visitor, Module *mod) {
  }


  AstDebugDirective::AstDebugDirective(const std::vector<Pointer<AstExpression>> &arguments,
    const std::vector<Token> &tokens,
    const SourceLocation &location)
    : AstDirectiveImpl(arguments, tokens, location) {
  }

  AstDebugDirective::~AstDebugDirective() {
  }

  void AstDebugDirective::visit(AstVisitor *visitor, Module *mod) {
    std::cout << "@debug says:  ";

    for (size_t i = 0; i < m_arguments.size(); i++) {
      auto arg = m_arguments[i];

      if (arg != nullptr) {
        arg->visit(visitor, mod);
      }

      std::cout << AstExpression::nodeToString(visitor, arg.get()) << "  ";
    }

    std::cout << "\n";
  }

  void AstDebugDirective::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
  }

  void AstDebugDirective::optimize(AstVisitor *visitor, Module *mod) {
  }


  AstUserDefinedDirective::AstUserDefinedDirective(const std::string &name,
    const std::vector<Pointer<AstExpression>> &arguments,
    const std::vector<Token> &tokens,
    const SourceLocation &location)
    : AstDirectiveImpl(arguments, tokens, location),
      m_name(name),
      m_iterator(nullptr),
      m_compilationUnit(nullptr) {
  }

  AstUserDefinedDirective::~AstUserDefinedDirective() {
    if (m_iterator != nullptr) {
      delete m_iterator;
    }

    if (m_compilationUnit != nullptr) {
      delete m_compilationUnit;
    }
  }

  void AstUserDefinedDirective::visit(AstVisitor *visitor, Module *mod) {
    // lookup macro data and instantiate it, re-parsing the body
    Macro *macro = visitor->getCompilationUnit()->getBoundGlobals().lookupMacro(m_name);

    if (!macro) {
      visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
        LEVEL_ERROR,
        Msg_custom_error,
        m_location,
        std::string("Could not find @macro with name `") + m_name + "`"
      ));

      return;
    }

    std::stringstream filenameStream;
    filenameStream << m_location.getFileName();
    filenameStream << "@" << m_name;
    filenameStream << " (instantiated on line " << m_location.getLine() << ")";

    TokenStream tokenStream(TokenStreamInfo { filenameStream.str() });

    for (auto &token : macro->getBody()) {
      tokenStream.push(token);
    }

    ASSERT(m_compilationUnit == nullptr);
    m_compilationUnit = new CompilationUnit(visitor->getCompilationUnit()->getDataStorage());

    m_compilationUnit->getBoundGlobals().setParent(
      &visitor->getCompilationUnit()->getBoundGlobals());

    // add variable for each argument.
    // (e.g _0 is m_arguments[0], _1 is m_arguments[1] and so on)
    // TODO: make an "args" var which is an Array that can be iterated upon
    for (size_t i = 0; i < m_arguments.size(); i++) {
      std::stringstream ss;
      ss << "_" << i;

      ASSERT(m_arguments[i] != nullptr);
      m_compilationUnit->getBoundGlobals().set(ss.str(), m_arguments[i]);

      // m_arguments[i]->visit(visitor, mod);

      // if (AstExpression *deepValue = m_arguments[i]->getDeepValueOf()) {
      //   if (Pointer<AstExpression> clonedExpr = std::dynamic_pointer_cast<AstExpression>(deepValue->clone())) {
      //     m_compilationUnit->getBoundGlobals().set(ss.str(), clonedExpr);
      //   }
      // }
    }

    m_compilationUnit->getBoundGlobals().set("body", Pointer<AstCodeBody>(
      new AstCodeBody(m_tokens, m_location)));

    ASSERT(m_iterator == nullptr);
    m_iterator = new AstIterator;

    Parser parser(m_iterator, &tokenStream, m_compilationUnit);
    parser.parse();

    Analyzer analyzer(m_iterator, m_compilationUnit);
    analyzer.analyze();

    for (auto &error : m_compilationUnit->getErrorList().getErrors()) {
      visitor->getCompilationUnit()->getErrorList().addError(error);
    }
  }

  void AstUserDefinedDirective::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
    ASSERT(m_iterator != nullptr);
    m_iterator->resetPosition();

    Compiler compiler(m_iterator, m_compilationUnit);

    std::unique_ptr<BytecodeChunk> sub(new BytecodeChunk);
    compiler.compile(sub.get(), false);
    out->append(std::move(sub));
  }

  void AstUserDefinedDirective::optimize(AstVisitor *visitor, Module *mod) {
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
    } else if (m_name == "get") {
      m_impl = new AstGetDirective(m_arguments, m_tokens, m_location);
    } else if (m_name == "set") {
      m_impl = new AstSetDirective(m_arguments, m_tokens, m_location);
    } else if (m_name == "yield") {
      m_impl = new AstYieldDirective(m_arguments, m_tokens, m_location);
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
