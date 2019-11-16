#pragma once

#include <string>

namespace bcparse {
  class SourceLocation {
  public:
    static const SourceLocation eof;

  public:
    SourceLocation(int line, int column, const std::string &filename);
    SourceLocation(const SourceLocation &other);

    inline int getLine() const { return m_line; }
    inline int &getLine() { return m_line; }
    inline int getColumn() const { return m_column; }
    inline int &getColumn() { return m_column; }
    inline const std::string &getFileName() const { return m_filename; }
    inline void setFileName(const std::string &filename) { m_filename = filename; }

    bool operator<(const SourceLocation &other) const;
    bool operator==(const SourceLocation &other) const;
    SourceLocation operator+(const SourceLocation &other);
    SourceLocation &operator+=(const SourceLocation &other);

  private:
    int m_line;
    int m_column;
    std::string m_filename;
  };
}
