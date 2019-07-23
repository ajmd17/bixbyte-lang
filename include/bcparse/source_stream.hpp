#pragma once

#include <cstddef>

#include <utf/utf8.hpp>
#include <bcparse/source_file.hpp>


namespace bcparse {
  class SourceStream {
  public:
      SourceStream(SourceFile *file);
      SourceStream(const SourceStream &other);

      inline SourceFile *getFile() const { return m_file; }
      inline size_t getPosition() const { return m_position; }
      inline bool hasNext() const { return m_position < m_file->getSize(); }
      utf::u32char peek() const;
      utf::u32char next();
      utf::u32char next(int &posChange);
      void goBack(int n = 1);
      void read(char *ptr, size_t numBytes);

  private:
      SourceFile *m_file;
      size_t m_position;
  };
}
