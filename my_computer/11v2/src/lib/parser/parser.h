#pragma once

#include "ast.h"
#include "tokenizer/jack_token.h"
#include "util/text_reader.h"

namespace jfcl {

class Parser {
public:
  Parser() = delete;
  Parser(const Parser&) = delete;
  Parser& operator=(const Parser&) = delete;

  Parser(Tokens_t&, AstTree& ast);

  void set_left_associative() { left_associative_expressions = true; }

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
  AstNodeRef create_ast_node(AstNodeType_t);

  template <typename T>
  AstNodeRef create_ast_node(AstNodeType_t type, T value);

  AstNodeRef parse_class(std::string& class_name);
  AstNodeRef parse_classvar_decl_block();
  AstNodeRef parse_subroutine();

  AstNodeRef parse_statement();
  AstNodeRef parse_subroutine_call();
  AstNodeRef parse_variable();
  AstNodeRef parse_term();

  AstNodeRef parse_expression();

  const AstTree& get_ast() { return AST; }

private:
  AstNodeRef parse_inner_statements();
  AstNodeRef parse_let_statement();
  AstNodeRef parse_do_statement();
  AstNodeRef parse_return_statement();
  AstNodeRef parse_while_statement();
  AstNodeRef parse_if_statement();
  AstNodeRef parse_type(AstNodeType_t);

  AstTree& AST;

  // support canonical Jack compiler's left associative operator binding
  bool left_associative_expressions {false};

  Tokens_t::iterator token_iter;
  Tokens_t::iterator token_iter_end;

  JackTokenCRef current_token;
  JackTokenCRef peek_token;
};

template <typename T>
AstNodeRef Parser::create_ast_node(AstNodeType_t type, T value)
{
  AstNodeRef ast_node = create_ast_node(type);
  ast_node.get().value = value;

  return ast_node;
}

}  // namespace jfcl
