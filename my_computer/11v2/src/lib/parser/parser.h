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

  Parser(Tokens_t&);

  void get_next_token()
  {
    current_token = peek_token;
    if (token_iter + 1 != token_iter_end)
    {
      peek_token = *(++token_iter);
    }
  }

  [[noreturn]] void fatal_error(std::string s) { throw std::runtime_error(s); }

  void require_token(TokenValue_t, TokenValue_t);
  void require_token(JackTokenCRef, TokenValue_t);

  // creates an orphaned AST node of the given type and optional value
  ast::AstNodeRef create_ast_node(ast::AstNodeType_t);

  template <typename T>
  ast::AstNodeRef create_ast_node(ast::AstNodeType_t type, T value);

  ast::AstNodeRef parse_class();
  ast::AstNodeRef parse_classvar_decl_block();
  ast::AstNodeRef parse_subroutine();

  ast::AstNodeRef parse_statement();
  ast::AstNodeRef parse_subroutine_call();
  ast::AstNodeRef parse_variable();

  ast::AstNodeRef parse_expression();
  ast::AstNodeRef parse_term();

private:
  ast::AstNodeRef parse_inner_statements();
  ast::AstNodeRef parse_let_statement();
  ast::AstNodeRef parse_do_statement();
  ast::AstNodeRef parse_return_statement();
  ast::AstNodeRef parse_while_statement();
  ast::AstNodeRef parse_if_statement();

  ast::AstTree AST;

  Tokens_t::iterator token_iter;
  Tokens_t::iterator token_iter_end;

  JackTokenCRef current_token;
  JackTokenCRef peek_token;

  // convention to represent an empty leaf
  const std::unique_ptr<ast::AstNode> EmptyNode;
  const ast::AstNodeRef EmptyNodeRef;
};

template <typename T>
ast::AstNodeRef Parser::create_ast_node(ast::AstNodeType_t type, T value)
{
  ast::AstNodeRef ast_node = create_ast_node(type);
  ast_node.get().value = value;

  return ast_node;
}

}  // namespace recursive_descent
