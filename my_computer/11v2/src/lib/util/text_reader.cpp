#include "text_reader.h"

#include <exception>
#include <sstream>

namespace jfcl {

TextReader::char_type TextReader::read()
{
  TextReader::char_type ch;

  if (cursor_pos <= raw_buffer.length())
  {
    ch = raw_buffer[cursor_pos++];
  }
  else
  {
    ch = '\0';
  }

  if (ch == '\n')
  {
    current_line_number++;
  }

  return ch;
}

TextReader::char_type TextReader::peek()
{
  if (cursor_pos <= raw_buffer.length())
  {
    return raw_buffer[cursor_pos];
  }
  return '\0';
}

TextReader::char_type TextReader::peek2()
{
  if (cursor_pos + 1 <= raw_buffer.length())
  {
    return raw_buffer[cursor_pos + 1];
  }
  return '\0';
}

void TextReader::init_buffer(const std::string& s)
{
  std::stringstream ss_line;

  for (char ch : s)
  {
    if (valid_ch(ch))
    {
      if (ch == '\n')
      {
        contents_store.emplace_back(
            std::make_unique<std::string>(ss_line.str()));
        auto& s_ptr_ref = contents_store.back();
        std::string_view s_view = *s_ptr_ref;
        contents.emplace_back(s_view);
        ss_line.str("");
        ss_line.clear();
      }
      else
      {
        ss_line << ch;
      }
    }
    else
    {
      // current position
      throw std::domain_error("Invalid character in input stream");
    }
  }

  if (!ss_line.str().empty())
  {
    contents_store.emplace_back(std::make_unique<std::string>(ss_line.str()));
    auto& s_ptr_ref = contents_store.back();
    std::string_view s_view = *s_ptr_ref;
    contents.emplace_back(s_view);
  }
  raw_buffer = s;
  cursor_pos = 0;
}

}  // namespace jfcl
