#include <bcparse/ast/ast_code_body.hpp>

#include <bcparse/lexer.hpp>
#include <bcparse/parser.hpp>
#include <bcparse/analyzer.hpp>
#include <bcparse/compiler.hpp>
#include <bcparse/ast_iterator.hpp>
#include <bcparse/ast_visitor.hpp>
#include <bcparse/compilation_unit.hpp>
#include <bcparse/bound_variables.hpp>
#include <bcparse/source_file.hpp>

#include <common/str_util.hpp>

#include <sstream>

namespace bcparse {
  AstCodeBody::AstCodeBody(const std::vector<Token> &tokens,
    const SourceLocation &location)
    : AstExpression(location),
      m_tokens(tokens),
      m_iterator(nullptr),
      m_compilationUnit(nullptr),
      m_variableMode(false) {
  }
  AstCodeBody::AstCodeBody(const std::vector<Token> &tokens,
    const SourceLocation &location,
    bool variableMode)
    : AstCodeBody(tokens, location) {
    m_variableMode = variableMode;
  }

  AstCodeBody::~AstCodeBody() {
    if (m_iterator != nullptr) {
      delete m_iterator;
      m_iterator = nullptr;
    }

    if (m_compilationUnit != nullptr) {
      delete m_compilationUnit;
      m_compilationUnit = nullptr;
    }
  }

  void AstCodeBody::visit(AstVisitor *visitor, Module *mod) {
    if (m_compilationUnit != nullptr && m_iterator != nullptr) {
      return;
    }

    TokenStream tokenStream(TokenStreamInfo { m_location.getFileName() });

    for (auto &token : m_tokens) {
      tokenStream.push(token);
    }

    ASSERT(m_compilationUnit == nullptr);
    m_compilationUnit = new CompilationUnit(visitor->getCompilationUnit()->getDataStorage());
    m_compilationUnit->setVariableMode(m_variableMode);

    m_compilationUnit->getBoundGlobals().setParent(
      &visitor->getCompilationUnit()->getBoundGlobals());

    ASSERT(m_iterator == nullptr);
    m_iterator = new AstIterator;

    Parser parser(m_iterator, &tokenStream, m_compilationUnit, m_variableMode);
    parser.parse();

    Analyzer analyzer(m_iterator, m_compilationUnit);
    analyzer.analyze();

    for (auto &error : m_compilationUnit->getErrorList().getErrors()) {
      visitor->getCompilationUnit()->getErrorList().addError(error);
    }
  }

  void AstCodeBody::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
    ASSERT(m_iterator != nullptr);
    m_iterator->resetPosition();

    Compiler compiler(m_iterator, m_compilationUnit);

    std::unique_ptr<BytecodeChunk> sub(new BytecodeChunk);
    compiler.compile(sub.get(), false);
    out->append(std::move(sub));
  }

  void AstCodeBody::optimize(AstVisitor *visitor, Module *mod) {
  }

  Pointer<AstStatement> AstCodeBody::clone() const {
    return CloneImpl();
  }

  std::string AstCodeBody::toString() const {
    std::stringstream ss;

    ss << "CodeBody(\"";

    for (const auto &token : m_tokens) {
      ss << str_util::escape_string(Token::getRepr(token));
      ss << ' ';
    }

    ss << "\")";

    return ss.str();
  }
}
