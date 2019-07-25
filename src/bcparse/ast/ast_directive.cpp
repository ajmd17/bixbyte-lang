#include <bcparse/ast/ast_directive.hpp>

#include <bcparse/lexer.hpp>
#include <bcparse/parser.hpp>
#include <bcparse/compilation_unit.hpp>
#include <bcparse/bound_variables.hpp>

#include <sstream>

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
    const size_t numErrors = visitor->getCompilationUnit().getErrorList().m_errors.size();

    if (m_arguments.size() != 1) {
      visitor->getCompilationUnit().getErrorList().addError(CompilerError(
        LEVEL_ERROR,
        Msg_custom_error,
        m_location,
        "@macro accepts arguments (name)"
      ));
    } else {
      if (Macro *macro = visitor->getCompilationUnit().lookupMacro(m_arguments.first())) {
        visitor->getCompilationUnit().getErrorList().addError(CompilerError(
          LEVEL_ERROR,
          Msg_custom_error,
          m_location,
          std::string("@macro `") + m_arguments.first() + "` already defined"
        ));
      }
    }

    if (m_body.length() == 0) {
      visitor->getCompilationUnit().getErrorList().addError(CompilerError(
        LEVEL_ERROR,
        Msg_custom_error,
        m_location,
        "@macro missing body"
      ));
    }

    if (visitor->getCompilationUnit().getErrorList().m_errors.size() > numErrors) return;

    visitor->getCompilationUnit().defineMacro(m_arguments.first(), body);
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
      m_name(name) {
  }
  
  AstUserDefinedDirective::~AstUserDefinedDirective() {
  }

  void AstUserDefinedDirective::visit(AstVisitor *visitor, Module *mod) {
    // lookup macro data and instantiate it, re-parsing the body
    Macro *macro = visitor->getCompilationUnit().lookupMacro(m_name);
    
    if (!macro) {
      visitor->getCompilationUnit().getErrorList().addError(CompilerError(
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

    CompilationUnit unit;
    unit.getBoundGlobals().setParent(&visitor->getCompilationUnit().getBoundGlobals());

    // add variable for each argument.
    // (e.g _0 is m_arguments[0], _1 is m_arguments[1] and so on)
    for (size_t i = 0; i < m_arguments.size(); i++) {
      std::stringstream ss;
      ss << "_" << i;

      unit.getBoundGlobals().set(ss.str(), m_arguments[i]);
    }

    unit.getBoundGlobals().set("body", Pointer<AstStringLiteral>(m_body, m_location));

    Lexer lexer(sourceStream, &tokenStream, &unit);
    lexer.analyze();

    // @TODO dadadada....
  }

  void AstUserDefinedDirective::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
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
    if (name == "macro") {
      m_impl = new AstMacroDirective(arguments, body, location);
    } else {
      // @TODO look through user-defined macros
    }

    if (m_impl != nullptr) {
      m_impl->visit(visitor, mod);
    } else {
      visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
        LEVEL_ERROR,
        Msg_unknown_directive,
        m_location,
        name
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
