#include <bcparse/ast/directives/ast_include_directive.hpp>

#include <bcparse/ast/ast_code_body.hpp>
#include <bcparse/ast/ast_string_literal.hpp>

#include <bcparse/lexer.hpp>
#include <bcparse/parser.hpp>
#include <bcparse/analyzer.hpp>
#include <bcparse/compiler.hpp>
#include <bcparse/ast_visitor.hpp>
#include <bcparse/ast_iterator.hpp>
#include <bcparse/source_file.hpp>
#include <bcparse/source_stream.hpp>
#include <bcparse/compilation_unit.hpp>

#include <common/str_util.hpp>

#include <fstream>

namespace bcparse {
  AstIncludeDirective::AstIncludeDirective(const std::vector<Pointer<AstExpression>> &arguments,
    const std::vector<Token> &tokens,
    const SourceLocation &location)
    : AstDirectiveImpl(arguments, tokens, location),
      m_iterator(nullptr),
      m_compilationUnit(nullptr) {
  }

  AstIncludeDirective::~AstIncludeDirective() {
    if (m_iterator != nullptr) {
      delete m_iterator;
    }

    if (m_compilationUnit != nullptr) {
      delete m_compilationUnit;
    }
  }

  void AstIncludeDirective::visit(AstVisitor *visitor, Module *mod) {
    AstStringLiteral *pathArg = nullptr;

    if (m_arguments.size() != 1) {
      visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
        LEVEL_ERROR,
        Msg_custom_error,
        m_location,
        "@include requires arguments (path)"
      ));
    } else {
      AstExpression *deepValue = nullptr;

      if (auto pathArgExpr = m_arguments[0].get()) {
        pathArgExpr->visit(visitor, mod);

        if ((deepValue = pathArgExpr->getDeepValueOf()) != nullptr) {
          pathArg = dynamic_cast<AstStringLiteral*>(deepValue);
        }
      }

      if (pathArg == nullptr) {
        if (m_arguments[0] != nullptr && deepValue != nullptr) {
          visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
            LEVEL_ERROR,
            Msg_custom_error,
            m_arguments[0]->getLocation(),
            "@include (path) must be a string, got %",
            AstExpression::nodeToString(visitor, deepValue)
          ));
        } else {
          visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
            LEVEL_ERROR,
            Msg_custom_error,
            m_location,
            "@include (path) must be an identifier"
          ));
        }
      } else {
        std::string currentDir;
        const size_t index = m_location.getFileName().find_last_of("/\\");
        if (index != std::string::npos) {
          currentDir = m_location.getFileName().substr(0, index) + "/";
        }

        const std::string pathValue = currentDir + pathArg->getValue();

            // parse path into vector
        std::vector<std::string> path_vec = str_util::split_path(pathValue);
        // canonicalize the vector
        path_vec = str_util::canonicalize_path(path_vec);
        // put it back into a string
        const std::string canon_path = str_util::path_to_str(path_vec);

        std::ifstream file;
        file.open(pathValue, std::ios::in | std::ios::ate);

        if (!file.is_open()) {
          visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
            LEVEL_ERROR,
            Msg_custom_error,
            m_location,
            "'%': path not found",
            pathValue
          ));

          return;
        }

        // get number of bytes
        size_t max = file.tellg();
        // seek to beginning
        file.seekg(0, std::ios::beg);
        // load stream into file buffer
        SourceFile sourceFile(pathValue, max);
        file.read(sourceFile.getBuffer(), max);

        SourceStream sourceStream(&sourceFile);
        TokenStream tokenStream(TokenStreamInfo { pathValue });

        ASSERT(m_compilationUnit == nullptr);
        m_compilationUnit = new CompilationUnit(visitor->getCompilationUnit()->getDataStorage());

        // m_compilationUnit->getBoundGlobals().setParent(
        //   &visitor->getCompilationUnit()->getBoundGlobals());

        ASSERT(m_iterator == nullptr);
        m_iterator = new AstIterator;

        Lexer lexer(sourceStream, &tokenStream, visitor->getCompilationUnit());
        lexer.analyze();

        Parser parser(m_iterator, &tokenStream, visitor->getCompilationUnit());
        parser.parse();

        Analyzer analyzer(m_iterator, visitor->getCompilationUnit());
        analyzer.analyze();

        // for (auto &error : m_compilationUnit->getErrorList().getErrors()) {
        //   visitor->getCompilationUnit()->getErrorList().addError(error);
        // }
      }
    }

  }

  void AstIncludeDirective::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
    ASSERT(m_iterator != nullptr);
    m_iterator->resetPosition();

    Compiler compiler(m_iterator, visitor->getCompilationUnit());

    std::unique_ptr<BytecodeChunk> sub(new BytecodeChunk);
    compiler.compile(sub.get(), false);
    out->append(std::move(sub));
  }

  void AstIncludeDirective::optimize(AstVisitor *visitor, Module *mod) {
  }
}
