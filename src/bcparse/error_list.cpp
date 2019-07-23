#include <unordered_set>
#include <map>
#include <fstream>

#include <common/str_util.hpp>
#include <common/termcolor.hpp>
#include <bcparse/error_list.hpp>

namespace bcparse {
  ErrorList::ErrorList() {
  }

  ErrorList::ErrorList(const ErrorList &other)
    : m_errors(other.m_errors) {
  }

  bool ErrorList::hasFatalErrors() const {
    if (!m_errors.empty()) {
      for (const CompilerError &error : m_errors) {
        if (error.getLevel() == LEVEL_ERROR) {
          return true;
        }
      }
    }

    return false;
  }

  std::ostream &ErrorList::writeOutput(std::ostream &os) const {
    std::unordered_set<std::string> error_filenames;
    std::vector<std::string> current_file_lines;

    for (const CompilerError &error : m_errors) {
      const std::string &path = error.getLocation().getFileName();

      if (error_filenames.insert(path).second) {
        current_file_lines.clear();
        // read lines into current_lines vector
        // this is used so that we can print out the line that an error occured on
        std::ifstream is(path);
        if (is.is_open()) {
          std::string line;
          while (std::getline(is, line)) {
            current_file_lines.push_back(line);
          }
        }

        auto split = str_util::split_path(path);
        std::string real_filename = !split.empty()
          ? split.back()
          : path;

        real_filename = str_util::strip_extension(real_filename);

        os << termcolor::reset << "In file \"" << real_filename << "\":\n";
      }

      const std::string &error_text = error.getText();

      switch (error.getLevel()) {
        case LEVEL_INFO:
          os << termcolor::white << termcolor::on_blue << termcolor::bold << "Info";
          break;
        case LEVEL_WARN:
          os << termcolor::white << termcolor::on_yellow << termcolor::bold << "Warning";
          break;
        case LEVEL_ERROR:
          os << termcolor::white << termcolor::on_red << termcolor::bold << "Error";
          break;
      }

      os << termcolor::reset
        << " at line "    << (error.getLocation().getLine() + 1)
        << ", col " << (error.getLocation().getColumn() + 1);

      os << ": " << error_text << '\n';

      if (current_file_lines.size() > error.getLocation().getLine()) {
        // render the line in question
        os << "\n\t" << current_file_lines[error.getLocation().getLine()];
        os << "\n\t";

        for (size_t i = 0; i < error.getLocation().getColumn(); i++) {
          os << ' ';
        }

        os << termcolor::green << "^";
      } else {
        os << '\t' << "<line not found>";
      }

      os << termcolor::reset << '\n';
    }

    return os;
  }
}
