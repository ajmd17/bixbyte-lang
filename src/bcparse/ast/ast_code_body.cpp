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
  AstCodeBody::AstCodeBody(const std::string &value, const SourceLocation &location)
    : AstExpression(location),
      m_value(value),
      m_iterator(nullptr),
      m_compilationUnit(nullptr) {
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
    // if (m_compilationUnit != nullptr && m_iterator != nullptr) {
    //   return;
    // }
    ASSERT(m_compilationUnit == nullptr);
    ASSERT(m_iterator == nullptr);

    SourceFile sourceFile(m_location.getFileName(), m_value.length());
    std::memcpy(sourceFile.getBuffer(), m_value.data(), m_value.length());

    SourceStream sourceStream(&sourceFile);
    TokenStream tokenStream(TokenStreamInfo { m_location.getFileName() });

    ASSERT(m_compilationUnit == nullptr);
    m_compilationUnit = new CompilationUnit(visitor->getCompilationUnit()->getDataStorage());

    m_compilationUnit->getBoundGlobals().setParent(
      &visitor->getCompilationUnit()->getBoundGlobals());

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
    return std::string("CodeBody(\"") + str_util::escape_string(m_value) + "\"";
  }
}
