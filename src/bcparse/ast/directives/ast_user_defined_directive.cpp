#include <bcparse/ast/directives/ast_user_defined_directive.hpp>

#include <bcparse/ast/ast_code_body.hpp>

#include <bcparse/lexer.hpp>
#include <bcparse/parser.hpp>
#include <bcparse/analyzer.hpp>
#include <bcparse/compiler.hpp>
#include <bcparse/ast_visitor.hpp>
#include <bcparse/ast_iterator.hpp>
#include <bcparse/compilation_unit.hpp>

namespace bcparse {
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
}
