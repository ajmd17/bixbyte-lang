#include <bcparse/token_stream.hpp>

namespace bcparse {
  TokenStream::TokenStream(const TokenStreamInfo &info)
    : m_position(0),
      m_info(info) {
  }
}
