#pragma once

#include <bcparse/token.hpp>
#include <common/my_assert.hpp>

#include <vector>

namespace bcparse {
  struct TokenStreamInfo {
    std::string filepath;

    TokenStreamInfo(const std::string &filepath)
      : filepath(filepath) {
    }

    TokenStreamInfo(const TokenStreamInfo &other)
      : filepath(other.filepath) {
    }
  };

  class TokenStream {
  public:
    TokenStream(const TokenStreamInfo &info);
    TokenStream(const TokenStream &other) = delete;

    inline Token peek(int n = 0) const {
        size_t pos = m_position + n;
        if (pos >= m_tokens.size()) {
            return Token::EMPTY;
        }
        return m_tokens[pos];
    }

    inline void push(const Token &token) { m_tokens.push_back(token); }
    inline bool hasNext() const { return m_position < m_tokens.size(); }
    inline Token next() { ASSERT(m_position < m_tokens.size()); return m_tokens[m_position++]; }
    inline Token rewind() { ASSERT(m_position > 0); return m_tokens[--m_position]; }
    inline Token last() const { ASSERT(!m_tokens.empty()); return m_tokens.back(); }
    inline size_t getSize() const { return m_tokens.size(); }
    inline size_t getPosition() const { return m_position; }
    inline const std::vector<Token> &getTokens() const { return m_tokens; }
    inline const TokenStreamInfo &getInfo() const { return m_info; }
    inline void setPosition(size_t position) { m_position = position; }
    inline bool eof() const { return m_position >= m_tokens.size(); }

    std::vector<Token> m_tokens;
    size_t m_position;

  private:
    TokenStreamInfo m_info;
  };
}
