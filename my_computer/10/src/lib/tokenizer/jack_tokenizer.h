#pragma once

#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "jack_lexical_elements.h"
#include "reader.h"

class JackTokenizer {
public:
  JackTokenizer() = delete;
  JackTokenizer(const JackTokenizer&) = delete;
  JackTokenizer& operator=(const JackTokenizer&) = delete;

  JackTokenizer(Reader& r) : reader(r) {}

  // get the token and its associated value
  JackToken get_token();

  // parse all tokens and return as a vector
  std::unique_ptr<std::vector<JackToken>> parse_tokens();

private:
  JackToken get_line_comment_token(char);
  JackToken get_block_comment_token(char);
  JackToken get_string_token(char);
  JackToken get_integer_token(char);
  JackToken get_jack_keyword_or_identifier_token(char);
  JackToken get_symbol_token(char);

  // used to build identifier strings or potential Jack keyword strings
  // since keywords are parsed first and can become identifiers (like "class1")
  bool valid_identifier_char(char ch)
  {
    bool r = false;
    r |= (ch == '_');
    r |= (ch >= '0') && (ch <= '9');
    ch |= 0x20;
    r |= (ch >= 'a') && (ch <= 'z');
    return r;
  };

  Reader& reader;
};
