#include <vector>
#include <string>
#include <utility>

#include <bcparse/lexer.hpp>

using Result = std::pair<bool, std::string>;

Result handleArgs(std::vector<std::string> args) {
  if (args.size() != 2) {
    return { false, std::string("Invalid arguments: expected `") + args[0] + " <filename>`" };
  }

  Result parseResult;

  return parseResult;
}

int main(int argc, char *argv[]) {
}
