#pragma once

#include <ctype.h>

#include <cstdint>
#include <exception>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace jfcl {

class TextReader {
public:
  using char_type = int8_t;

  TextReader() = default;

  char_type read();
  char_type peek();
  char_type peek2();
  bool eof() { return cursor_pos >= raw_buffer.size(); }
  size_t num_lines() { return contents.size(); }

  TextReader(const TextReader&) = delete;
  TextReader& operator=(const TextReader&) = delete;
  TextReader(TextReader&&) = delete;
  TextReader& operator=(TextReader&&) = delete;

  inline TextReader(const char* s) { init_buffer(s); }
  inline TextReader(std::string s) { init_buffer(s.data()); }

  [[nodiscard]] const std::string_view get_line(size_t x) const
  {
    if (x >= contents.size())
      throw std::runtime_error("Read beyond input");

    return contents[static_cast<Contents_t::size_type>(x)];
  }

  bool valid_ch(char ch) { return isascii(ch) && (isspace(ch) | isprint(ch)); }
  int get_current_line_number() { return current_line_number; }

protected:
  void init_buffer(const char*);

  using Contents_t = std::vector<std::string>;
  Contents_t contents;

  std::string raw_buffer;
  size_t cursor_pos {0};
  int current_line_number {1};
};

}  // namespace jfcl
