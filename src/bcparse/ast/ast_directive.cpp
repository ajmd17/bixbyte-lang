#include <bcparse/ast/ast_directive.hpp>
#include <bcparse/ast/ast_string_literal.hpp>

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

#include <sstream>
#include <iostream>

namespace bcparse {
  AstDirectiveImpl::AstDirectiveImpl(const std::vector<Pointer<AstExpression>> &arguments,
    const std::string &body,
    const SourceLocation &location)
    : m_arguments(arguments),
      m_body(body),
      m_location(location) {
  }

  AstMacroDirective::AstMacroDirective(const std::vector<Pointer<AstExpression>> &arguments,
    const std::string &body,
    const SourceLocation &location)
    : AstDirectiveImpl(arguments, body, location) {
  }

  AstMacroDirective::~AstMacroDirective() {
  }

  void AstMacroDirective::visit(AstVisitor *visitor, Module *mod) {
    const size_t numErrors = visitor->getCompilationUnit()->getErrorList().m_errors.size();

    AstStringLiteral *nameArg = nullptr;

    if (m_arguments.size() != 1) {
      visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
        LEVEL_ERROR,
        Msg_custom_error,
        m_location,
        "@macro accepts arguments (name)"
      ));
    } else {
      nameArg = dynamic_cast<AstStringLiteral*>(m_arguments.front().get());

      if (nameArg == nullptr) {
        visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
          LEVEL_ERROR,
          Msg_custom_error,
          m_location,
          "@macro (name) must be a string"
        ));
      } else {
        if (Macro *macro = visitor->getCompilationUnit()->lookupMacro(nameArg->getValue())) {
          visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
            LEVEL_ERROR,
            Msg_custom_error,
            m_location,
            std::string("@macro `") + nameArg->getValue() + "` already defined"
          ));
        }
      }
    }

    if (m_body.length() == 0) {
      visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
        LEVEL_ERROR,
        Msg_custom_error,
        m_location,
        "@macro missing body"
      ));
    }

    if (visitor->getCompilationUnit()->getErrorList().m_errors.size() > numErrors) return;

    visitor->getCompilationUnit()->defineMacro(nameArg->getValue(), m_body);
  }

  void AstMacroDirective::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
  }

  void AstMacroDirective::optimize(AstVisitor *visitor, Module *mod) {
  }



  AstUserDefinedDirective::AstUserDefinedDirective(const std::string &name,
    const std::vector<Pointer<AstExpression>> &arguments,
    const std::string &body,
    const SourceLocation &location)
    : AstDirectiveImpl(arguments, body, location),
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
    Macro *macro = visitor->getCompilationUnit()->lookupMacro(m_name);

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

    std::string macroBody = macro->getBody();

    SourceFile sourceFile(filenameStream.str(), macroBody.length());
    std::memcpy(sourceFile.getBuffer(), macroBody.data(), macroBody.length());

    SourceStream sourceStream(&sourceFile);
    TokenStream tokenStream(TokenStreamInfo { filenameStream.str() });

    ASSERT(m_compilationUnit == nullptr);
    m_compilationUnit = new CompilationUnit(visitor->getCompilationUnit()->getDataStorage());

    m_compilationUnit->getBoundGlobals().setParent(
      &visitor->getCompilationUnit()->getBoundGlobals());

    // add variable for each argument.
    // (e.g _0 is m_arguments[0], _1 is m_arguments[1] and so on)
    for (size_t i = 0; i < m_arguments.size(); i++) {
      std::stringstream ss;
      ss << "_" << i;

      m_compilationUnit->getBoundGlobals().set(ss.str(), m_arguments[i]);
    }

    m_compilationUnit->getBoundGlobals().set("body", Pointer<AstStringLiteral>(
      new AstStringLiteral(m_body, m_location)));

    Lexer lexer(sourceStream, &tokenStream, m_compilationUnit);
    lexer.analyze();

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
    const std::string &body,
    const SourceLocation &location)
    : AstStatement(location),
      m_name(name),
      m_arguments(arguments),
      m_body(body),
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
      m_impl = new AstMacroDirective(m_arguments, m_body, m_location);
    } else if (visitor->getCompilationUnit()->lookupMacro(m_name)) {
      m_impl = new AstUserDefinedDirective(m_name, m_arguments, m_body, m_location);
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
}
