#include <bcparse/source_stream.hpp>

#include <stdexcept>

namespace bcparse {
  SourceStream::SourceStream(SourceFile *file)
      : m_file(file),
        m_position(0)
  {
  }

  SourceStream::SourceStream(const SourceStream &other)
      : m_file(other.m_file),
        m_position(other.m_position)
  {
  }

  utf::u32char SourceStream::peek() const
  {
      size_t pos = m_position;

      if (pos >= m_file->getSize()) {
          return '\0';
      }

      // the current character
      char ch = m_file->getBuffer()[pos];

      // the character as a utf-32 character
      utf::u32char u32_ch = 0;
      char *bytes = utf::get_bytes(u32_ch);

      // check to see if it is a utf-8 character
      const unsigned char uc = (unsigned char)ch;

      if (uc >= 0 && uc <= 127) {
          // 1-byte character
          bytes[0] = ch;
      } else if ((uc & 0xE0) == 0xC0) {
          // 2-byte character
          bytes[0] = ch;
          bytes[1] = m_file->getBuffer()[pos + 1];
      } else if ((uc & 0xF0) == 0xE0) {
          // 3-byte character
          bytes[0] = ch;
          bytes[1] = m_file->getBuffer()[pos + 1];
          bytes[2] = m_file->getBuffer()[pos + 2];
      } else if ((uc & 0xF8) == 0xF0) {
          // 4-byte character
          bytes[0] = ch;
          bytes[1] = m_file->getBuffer()[pos + 1];
          bytes[2] = m_file->getBuffer()[pos + 2];
          bytes[3] = m_file->getBuffer()[pos + 3];
      } else {
          // invalid utf-8
          u32_ch = (utf::u32char)('\0');
      }

      return u32_ch;
  }

  utf::u32char SourceStream::next()
  {
      int tmp;
      return next(tmp);
  }

  utf::u32char SourceStream::next(int &posChange)
  {
      int posBefore = m_position;

      if (m_position >= m_file->getSize()) {
          return '\0';
      }

      // the current character
      char ch = m_file->getBuffer()[m_position++];

      // the character as a utf-32 character
      utf::u32char u32_ch = 0;
      char *bytes = utf::get_bytes(u32_ch);

      // check to see if it is a utf-8 character
      const unsigned char uc = (unsigned char)ch;
      
      if (uc >= 0 && uc <= 127) {
          // 1-byte character
          bytes[0] = ch;
      } else if ((uc & 0xE0) == 0xC0) {
          // 2-byte character
          bytes[0] = ch;
          bytes[1] = m_file->getBuffer()[m_position++];
      } else if ((uc & 0xF0) == 0xE0) {
          // 3-byte character
          bytes[0] = ch;
          bytes[1] = m_file->getBuffer()[m_position++];
          bytes[2] = m_file->getBuffer()[m_position++];
      } else if ((uc & 0xF8) == 0xF0) {
          // 4-byte character
          bytes[0] = ch;
          bytes[1] = m_file->getBuffer()[m_position++];
          bytes[2] = m_file->getBuffer()[m_position++];
          bytes[3] = m_file->getBuffer()[m_position++];
      } else {
          // invalid utf-8
          u32_ch = (utf::u32char)('\0');
      }

      pos_change = m_position - pos_before;

      return u32_ch;
  }

  void SourceStream::goBack(int n)
  {
      if (((int)m_position - n) < 0) {
          throw std::out_of_range("not large enough to go back");
      }
      m_position -= n;
  }

  void SourceStream::read(char *ptr, size_t numBytes)
  {
      for (size_t i = 0; i < numBytes; i++) {
          if (m_position >= m_file->getSize()) {
              throw std::out_of_range("attempted to read past the limit");
          }
          
          ptr[i] = m_file->getBuffer()[m_position++];
      }
  }
}
