#pragma once

#include <string>
#include <sstream>
#include <map>

#include <bcparse/source_location.hpp>

namespace bcparse {
  enum ErrorLevel {
    LEVEL_INFO,
    LEVEL_WARN,
    LEVEL_ERROR
  };

  enum ErrorMessage {};

  class CompilerError {
    static const std::map<ErrorMessage, std::string> error_message_strings;

  public:
    template <typename...Args>
    CompilerError(ErrorLevel level,
      ErrorMessage msg,
      const SourceLocation &location,
      const Args &...args)
        : m_level(level),
          m_msg(msg),
          m_location(location) {
      std::string msg_str = error_message_strings.at(m_msg);
      MakeMessage(msg_str.c_str(), args...);
    }

    CompilerError(const CompilerError &other);
    ~CompilerError() {}

    inline ErrorLevel getLevel() const { return m_level; }
    inline ErrorMessage getMessage() const { return m_msg; }
    inline const SourceLocation &getLocation() const { return m_location; }
    inline const std::string &getText() const { return m_text; }

    bool operator<(const CompilerError &other) const;

  private:
    void makeMessage(const char *format) { m_text += format; }

    template <typename T, typename ... Args>
    void makeMessage(const char *format, T value, Args && ... args) {
      for (; *format; format++) {
        if (*format == '%') {
          std::stringstream sstream;
          sstream << value;

          m_text += sstream.str();

          makeMessage(format + 1, args...);

          return;
        }

        m_text += *format;
      }
    }

    ErrorLevel m_level;
    ErrorMessage m_msg;
    SourceLocation m_location;
    std::string m_text;
  };
}
