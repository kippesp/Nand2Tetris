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
  Parser(JackTokenizer::Tokens_t&);

  void get_next_token()
  {
    current_token = peek_token;
    if (token_iter + 1 != token_iter_end)
    {
      peek_token = *(++token_iter);
    }
  }

  [[noreturn]] void fatal_error(std::string s) { throw std::runtime_error(s); }

  void require_token(TokenValue_t start_token, TokenType_t token_type);
  void require_token(TokenValue_t start_token, TokenValue_t token_value);

  // creates an orphaned AST node of the given type and optional value
  ast::AstNodeRef create_ast_node(ast::AstNodeType_t);

  template <typename T>
  ast::AstNodeRef create_ast_node(ast::AstNodeType_t type, T value);

  ast::AstNodeRef parse_class();
  ast::AstNodeRef parse_subroutine();
  ast::AstNodeRef parse_let_statement();
  ast::AstNodeRef parse_return_statement();
  ast::AstNodeRef parse_expression();

  // Returns the root node of the AST
  // const ast::AstNode& get_ast

private:
  ast::AstTree AST;

  JackTokenizer::Tokens_t::iterator token_iter;
  JackTokenizer::Tokens_t::iterator token_iter_end;

  std::reference_wrapper<const JackToken> current_token;
  std::reference_wrapper<const JackToken> peek_token;
};

template <typename T>
ast::AstNodeRef Parser::create_ast_node(ast::AstNodeType_t type, T value)
{
  ast::AstNodeRef ast_node = create_ast_node(type);
  ast_node.get().value = value;

  return ast_node;
}

}  // namespace recursive_descent
