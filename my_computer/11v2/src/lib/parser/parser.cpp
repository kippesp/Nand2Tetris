#include "parser.h"

#include <cassert>

#include <signal.h>

using namespace ast;

namespace recursive_descent {

Parser::Parser(JackTokenizer::Tokens_t& tokens)
    : token_iter(tokens.begin()),
      current_token(*token_iter),
      peek_token(*(++token_iter))
{
  // TODO: create empty ast
  // raise(SIGTRAP);
}

void Parser::require_token(TokenValue_t start_token, TokenType_t token_type)
{
  if (current_token.get().type != token_type)
  {
    std::stringstream ss;

    ss << "Line #" << current_token.get().line_number << ": "
       << "Expected " << JackToken::to_string(token_type) << " while parsing "
       << JackToken::to_string(start_token);

    fatal_error(ss.str());
  }
}

void Parser::require_token(TokenValue_t start_token, TokenValue_t token_value)
{
  if (current_token.get().value_enum != token_value)
  {
    std::stringstream ss;

    ss << "Line #" << current_token.get().line_number << ": "
       << "Expected " << JackToken::to_string(token_value) << " while parsing "
       << JackToken::to_string(start_token);

    fatal_error(ss.str());
  }
}

AstNodeRef Parser::create_ast_node(AstNodeType_t type)
{
  return AST.add(AstNode(type, current_token));
}

AstNodeRef Parser::parse_let_statement()
{
  const auto start_token = TokenValue_t::J_LET;

  assert(current_token.get().value_enum == start_token);

  AstNodeRef LetAst = create_ast_node(AstNodeType_t::N_LET_STATEMENT);

  get_next_token();

  require_token(start_token, TokenType_t::J_IDENTIFIER);

  // Array variable
  if (peek_token.get().value_enum == TokenValue_t::J_LEFT_BRACKET)
  {
    assert(0 && "Array variable support required");
  }
  // Scalar variable
  else
  {
    AstNodeRef ScalarVarAst = create_ast_node(AstNodeType_t::N_SCALAR_VAR);
    AstNodeRef VarNameAst = create_ast_node(AstNodeType_t::N_VARIABLE_NAME,
                                            current_token.get().value_str);

    ScalarVarAst.get().add_child(VarNameAst);
    LetAst.get().add_child(ScalarVarAst);
  }

  get_next_token();

  require_token(start_token, TokenValue_t::J_EQUAL);

  get_next_token();

  AstNode& ExpressionAst = parse_expression();
  LetAst.get().add_child(ExpressionAst);

  require_token(start_token, TokenValue_t::J_SEMICOLON);

  return LetAst;
}

AstNodeRef Parser::parse_expression()
{
  AstNodeRef ExpressionAst = create_ast_node(AstNodeType_t::N_EXPRESSION);

  if (current_token.get().value_enum == TokenValue_t::J_TRUE)
  {
    AstNodeRef TermAst = create_ast_node(AstNodeType_t::N_TERM);
    AstNodeRef KeywordConstAst =
        create_ast_node(AstNodeType_t::N_KEYWORD_CONSTANT);
    AstNodeRef TrueKeywordAst = create_ast_node(AstNodeType_t::N_TRUE_KEYWORD);

    KeywordConstAst.get().add_child(TrueKeywordAst);
    TermAst.get().add_child(KeywordConstAst);

    ExpressionAst.get().add_child(TermAst);

    get_next_token();
  }
  else
  {
    assert(0 && "expression failed to parse");
  }

  return ExpressionAst;
}

}  // namespace recursive_descent
