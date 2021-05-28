#pragma once

#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "jack_token.h"
#include "util/text_reader.h"

class JackTokenizer {
public:
  JackTokenizer() = delete;
  JackTokenizer(const JackTokenizer&) = delete;
  JackTokenizer& operator=(const JackTokenizer&) = delete;

  JackTokenizer(TextReader& r) : reader(r) {}

  using Tokens_t = std::vector<JackToken>;

  // parse all tokens and return as an std::vector*
  const Tokens_t* parse_tokens();

protected:
  // used to build identifier strings or potential Jack keyword strings
  // since keywords are parsed first and can become identifiers (like "class1")
  bool valid_identifier_char(char);

private:
  JackToken get_line_comment_token(char);
  JackToken get_block_comment_token(char);
  JackToken get_string_token(char);
  JackToken get_integer_token(char);
  JackToken get_jack_keyword_or_identifier_token(char);
  JackToken get_symbol_token(char);

  // get the token and its associated value
  JackToken get_next_token();

  std::unique_ptr<Tokens_t> token_vect{nullptr};

  TextReader& reader;
};
