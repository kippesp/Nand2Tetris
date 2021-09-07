#pragma once

#include "ast.h"
#include "tokenizer/jack_tokenizer.h"

namespace recursive_descent {

// Builds an AST by parsing the tokenized input

class Parser {
public:
  Parser() = delete;
  Parser(const Parser&) = delete;
  Parser& operator=(const Parser&) = delete;

  // Parser(const JackTokenizer::Tokens_t*);
  Parser(const JackTokenizer::Tokens_t&);

  // Returns the root node of the AST
  // const ast::AstNode& get_ast

private:
  JackTokenizer::Tokens_t::iterator token_iter;

  const JackToken& current_token;
  const JackToken& next_token;
};

}  // namespace recursive_descent
