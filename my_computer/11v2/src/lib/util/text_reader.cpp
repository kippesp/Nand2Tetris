#include "text_reader.h"

#include <exception>
#include <sstream>

TextReader::char_type TextReader::read()
{
  TextReader::char_type ch;

  if (cursor_pos <= raw_buffer.length())
    ch = raw_buffer[cursor_pos++];
  else
    ch = '\0';

  if (ch == '\n')
    current_line_number++;

  return ch;
}

TextReader::char_type TextReader::peek()
{
  if (cursor_pos <= raw_buffer.length())
    return raw_buffer[cursor_pos];
  else
    return '\0';
}

void TextReader::init_buffer(const char* buf)
{
  std::stringstream ss_line;
  std::string s(buf);

  for (char ch : s)
  {
    if (valid_ch(ch))
    {
      if (ch == '\n')
      {
        contents.push_back(ss_line.str());
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
    contents.push_back(ss_line.str());
  raw_buffer = s;
  cursor_pos = 0;
}

TextReader::TextReader(const char* buf)
{
  init_buffer(buf);
}
