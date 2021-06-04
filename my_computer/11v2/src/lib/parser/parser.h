#pragma once

#include "tokenizer/jack_tokenizer.h"

class Parser {
public:
  Parser() = delete;
  Parser(const Parser&) = delete;
  Parser& operator=(const Parser&) = delete;

  Parser(const JackTokenizer::Tokens_t*);

private:
}
