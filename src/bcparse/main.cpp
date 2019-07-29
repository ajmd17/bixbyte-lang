#include <vector>
#include <string>
#include <utility>
#include <memory>

#include <bcparse/emit/bytecode_chunk.hpp>

#include <bcparse/lexer.hpp>
#include <bcparse/compilation_unit.hpp>
#include <bcparse/source_file.hpp>
#include <bcparse/token_stream.hpp>
#include <bcparse/ast_iterator.hpp>
#include <bcparse/parser.hpp>
#include <bcparse/analyzer.hpp>
#include <bcparse/compiler.hpp>
#include <bcparse/emit/emitter.hpp>

#include <common/clarg.hpp>
#include <common/str_util.hpp>

using namespace bcparse;

using Result = std::pair<bool, std::string>;
using UStr = utf::Utf8String;

namespace bcparse {
  class CompilerHelper {
  public:
    static Result buildSourceFile(UStr filename, CompilationUnit *unit, BytecodeChunk *out) {
      std::stringstream ss;
      
      std::ifstream in_file(
        filename.GetData(),
        std::ios::in | std::ios::ate | std::ios::binary
      );

      if (!in_file.is_open()) {
        utf::cout << "Could not open file: " << filename << "\n";
      } else {
        // get number of bytes
        size_t max = in_file.tellg();
        // seek to beginning
        in_file.seekg(0, std::ios::beg);
        // load stream into file buffer
        SourceFile sourceFile(filename.GetData(), max);
        in_file.read(sourceFile.getBuffer(), max);
        in_file.close();

        SourceStream sourceStream(&sourceFile);

        TokenStream tokenStream(TokenStreamInfo {
          std::string(filename.GetData())
        });

        Lexer lex(sourceStream, &tokenStream, unit);
        lex.analyze();

        AstIterator iterator;
        Parser parser(&iterator, &tokenStream, unit);
        parser.parse();

        Analyzer analyzer(&iterator, unit);
        analyzer.analyze();


        // SemanticAnalyzer semantic_analyzer(&ast_iterator, &compilation_unit);
        // semantic_analyzer.Analyze();

        unit->getErrorList().sortErrors();
        unit->getErrorList().writeOutput(ss); // TODO make utf8 compatible

        if (!unit->getErrorList().hasFatalErrors()) {
          // only optimize if there were no errors
          // before this point

          // ast_iterator.ResetPosition();
          // Optimizer optimizer(&ast_iterator, &compilation_unit);
          // optimizer.Optimize();

          // compile into bytecode instructions
          iterator.resetPosition();
          
          Compiler compiler(&iterator, unit);

          std::unique_ptr<BytecodeChunk> chunk(new BytecodeChunk);
          compiler.compile(chunk.get());
          out->append(std::move(chunk));

          // @TODO write bytecode file

          // ast_iterator.ResetPosition();
          // Compiler compiler(&ast_iterator, &compilation_unit);
          // return compiler.Compile();

          return { true, "" };
        }
      }

      return { false, ss.rdbuf()->str() };
    }
  };
}

Result handleArgs(int argc, char *argv[]) {
  if (argc != 2) {
    return { false, std::string("Invalid arguments: expected `") + argv[0] + " <filename>`" };
  }

  Result parseResult;

  UStr inFilename, outFilename;

  if (Clarg::has(argv, argv + argc, "-c")) {
    inFilename = Clarg::get(argv, argv + argc, "-c");
  }

  if (inFilename == "") {
    inFilename = argv[argc - 1];
  }

  if (Clarg::has(argv, argv + argc, "-o")) {
    outFilename = Clarg::get(argv, argv + argc, "-o");
  }

  if (outFilename == "") {
    outFilename = (str_util::strip_extension(inFilename.GetData()) + ".bin").c_str();
  }

  BytecodeChunk chunk;
  DataStorage dataStorage;
  CompilationUnit unit(&dataStorage);

  Result r = CompilerHelper::buildSourceFile(inFilename, &unit, &chunk);

  if (!r.first) {
    return r;
  }

  std::ofstream of(outFilename.GetData(), std::ios::out | std::ios::binary);

  if (!of.is_open()) {
    std::stringstream ss;
    ss << "Could not write to output file: ";
    ss << outFilename.GetData();

    return { false, ss.str() };
  }


  Emitter emitter(&chunk);
  emitter.emit(&of);

  return { true, "" };
}

int main(int argc, char *argv[]) {
  Result r = handleArgs(argc, argv);

  switch (r.first) {
    case true:
      return 0;
    case false:
      utf::cout << r.second << std::endl;
      return 1;
  }
}
