#pragma once

#include <ctype.h>

#include <cstdint>
#include <exception>
#include <string>
#include <string_view>
#include <vector>

class TextReader {
public:
  using char_type = int8_t;

  TextReader() = default;

  char_type read();
  char_type peek();
  bool eof() { return cursor_pos >= raw_buffer.size(); }
  size_t num_lines() { return contents.size(); }

  TextReader(const TextReader&) = delete;
  TextReader& operator=(const TextReader&) = delete;
  TextReader(TextReader&&) = delete;
  TextReader& operator=(TextReader&&) = delete;

  TextReader(const char*);

  [[nodiscard]] const std::string_view get_line(size_t x) const
  {
    if (x >= contents.size())
      throw std::overflow_error("Read beyond input");

    return contents[static_cast<Contents_t::size_type>(x)];
  }

  bool valid_ch(char ch) { return isascii(ch) && (isspace(ch) | isprint(ch)); }
  size_t get_current_line_number() { return current_line_number; }

protected:
  void init_buffer(const char*);

  using Contents_t = std::vector<std::string>;
  Contents_t contents;

  std::string raw_buffer;
  size_t cursor_pos {0};
  size_t current_line_number {1};
};
