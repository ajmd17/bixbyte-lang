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
    std::stringstream filenameStream;
    filenameStream << m_location.getFileName();
    filenameStream << "@" << m_name;

    SourceFile sourceFile(filenameStream.str(), m_body.size());
    std::memcpy(sourceFile.getBuffer(), m_body.data(), m_body.size());

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

    Lexer lexer(sourceStream, &tokenStream, &unit);
    lexer.analyze();

    // @TODO dadadada....
  }

  void AstMacroDirective::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
  }

  void AstMacroDirective::optimize(AstVisitor *visitor, Module *mod) {
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
