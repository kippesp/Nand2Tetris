#include <string.h>

#include "catch.hpp"
#include "mock_reader.h"
#include "parser/jack_ast.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// Tests for Jack Expressions
/////////////////////////////////////////////////////////////////////////////

#if 0
SCENARIO("Parse Jack term")
{
  test::MockReader R;

  SECTION("simple integers")
  {
    strcpy(R.buffer, "1");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    JackAst T(tokens);

    auto node = T.parse_expression();

    REQUIRE(node->type == AstNodeType_t::N_INTEGER_CONSTANT);
    REQUIRE(node->token.value_str == "1");
  }

  SECTION("simple variable")
  {
    strcpy(R.buffer, "class1");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    JackAst T(tokens);

    auto node = T.parse_expression();

    REQUIRE(node->type == AstNodeType_t::N_VARIABLE);
    REQUIRE(node->token.value_str == "class1");
  }

  SECTION("simple string")
  {
    strcpy(R.buffer, "\"a string\"");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    JackAst T(tokens);

    auto node = T.parse_expression();

    REQUIRE(node->type == AstNodeType_t::N_STRING_CONSTANT);
    REQUIRE(node->token.value_str == "a string");
  }

  SECTION("keyword constant - true")
  {
    strcpy(R.buffer, "true");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    JackAst T(tokens);

    auto node = T.parse_expression();

    REQUIRE(node->type == AstNodeType_t::N_KEYWORD_CONSTANT);
    REQUIRE(node->token.value_enum == TokenValue_t::J_TRUE);
  }

  SECTION("keyword constant - false")
  {
    strcpy(R.buffer, "false");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    JackAst T(tokens);

    auto node = T.parse_expression();

    REQUIRE(node->type == AstNodeType_t::N_KEYWORD_CONSTANT);
    REQUIRE(node->token.value_enum == TokenValue_t::J_FALSE);
  }

  SECTION("keyword constant - null")
  {
    strcpy(R.buffer, "null");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    JackAst T(tokens);

    auto node = T.parse_expression();

    REQUIRE(node->type == AstNodeType_t::N_KEYWORD_CONSTANT);
    REQUIRE(node->token.value_enum == TokenValue_t::J_NULL);
  }

  SECTION("keyword constant - this")
  {
    strcpy(R.buffer, "this");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    JackAst T(tokens);

    auto node = T.parse_expression();

    REQUIRE(node->type == AstNodeType_t::N_KEYWORD_CONSTANT);
    REQUIRE(node->token.value_enum == TokenValue_t::J_THIS);
  }
}
#endif
