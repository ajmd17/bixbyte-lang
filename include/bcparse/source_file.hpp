#pragma once

namespace bcparse {
  #include <string>
  #include <cstddef>

  class SourceFile {
  public:
      SourceFile(const std::string &filepath, size_t size);
      SourceFile(const SourceFile &other) = delete;
      ~SourceFile();

      SourceFile &operator=(const SourceFile &other) = delete;

      // input into buffer
      SourceFile &operator>>(const std::string &str);

      inline const std::string &getFilePath() const { return m_filepath; }
      inline char *getBuffer() const { return m_buffer; }
      inline size_t getSize() const { return m_size; }
      inline void setSize(size_t size) { m_size = size; }
      
      void readIntoBuffer(const char *data, size_t size);

  private:
      std::string m_filepath;
      char *m_buffer;
      size_t m_position;
      size_t m_size;
  };
}
