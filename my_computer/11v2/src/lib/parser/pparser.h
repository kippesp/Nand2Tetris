#pragma once

#include "ast.h"
#include "tokenizer/jack_tokenizer.h"

#include <functional>
#include <map>

namespace pratt {

// Builds an AST by parsing the tokenized input

// using JackTokenRef = std::reference_wrapper<JackToken>;
// using JackTokenCRef = std::reference_wrapper<const JackToken>;
// using Tokens_t = std::vector<JackTokenCRef>;
// using ParserFn_t = std::function<ast::AstNodeRef(JackTokenCRef, int)>;

// using PrefixParserFn_t = std::function<ast::AstNodeRef()>;
// using InfixParserFn_t = std::function<ast::AstNodeRef(ast::AstNodeRef)>;

// using ParserFnTuple_t = std::tuple<InfixParserFn_t, PrefixParserFn_t>;

// using TokenParsers_t = std::tuple<ParserFn_t, ParserFn_t, int>;

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

  void require_token(TokenValue_t start_token, TokenValue_t token_value);

  // creates an orphaned AST node of the given type and optional value
  ast::AstNodeRef create_ast_node(ast::AstNodeType_t);

  template <typename T>
  ast::AstNodeRef create_ast_node(ast::AstNodeType_t type, T value);

  ast::AstNodeRef parse_expression();

private:
  // Pratt parser function types and precedence alias
  using PrefixParserFn_t = std::function<ast::AstNodeRef()>;
  using InfixParserFn_t = std::function<ast::AstNodeRef(ast::AstNodeRef)>;
  using precedence_t = uint32_t;

  using TokenParsers_t =
      std::tuple<PrefixParserFn_t, InfixParserFn_t, precedence_t>;
  using ParserMap_t = std::map<TokenValue_t, TokenParsers_t>;

  ast::AstNodeRef prefixParser();
  ast::AstNodeRef infixParser(ast::AstNodeRef);

  ast::AstNodeRef integer_handler();

  ast::AstNodeRef parse_expression(precedence_t);

  Tokens_t::iterator token_iter;
  Tokens_t::iterator token_iter_end;

  std::reference_wrapper<const JackToken> current_token;
  std::reference_wrapper<const JackToken> peek_token;

  ParserMap_t ParserMap;

  ast::AstTree AST;

  ast::AstNode EmptyNode {ast::AstNodeType_t::N_UNDEFINED};
  ast::AstNodeRef EmptyNodeRef {EmptyNode};
};

template <typename T>
ast::AstNodeRef Parser::create_ast_node(ast::AstNodeType_t type, T value)
{
  ast::AstNodeRef ast_node = create_ast_node(type);
  ast_node.get().value = value;

  return ast_node;
}

}  // namespace pratt
