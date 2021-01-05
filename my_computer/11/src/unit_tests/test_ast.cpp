#include <string.h>

#include "catch.hpp"
#include "mock_reader.h"
#include "parser/jack_ast.h"

using namespace std;

SCENARIO("AST Builder")
{
  test::MockReader R;

  SECTION("simple node builder operations")
  {
    strcpy(R.buffer, "1");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    JackAst T(AstNodeType_t::N_TERM, tokens);

    REQUIRE((*tokens)[0].type == TokenType_t::J_INTEGER_CONSTANT);
    REQUIRE((*tokens)[0].value_str == "1");

    // <term>
    auto root_node = T.get_root();

    REQUIRE(root_node->type == AstNodeType_t::N_TERM);
    REQUIRE(root_node->num_child_nodes() == 0);

    //         <term>
    //           |
    //  <integer-constant>:1

    auto child = root_node->create_child(AstNodeType_t::N_INTEGER_CONSTANT,
                                         (*tokens)[0]);

    REQUIRE(child->type == AstNodeType_t::N_INTEGER_CONSTANT);
    REQUIRE(child->token.value_str == "1");

    auto child_nodes = root_node->get_child_nodes();

    REQUIRE(child_nodes[0] == child);
    REQUIRE(root_node->num_child_nodes() == 1);
  }

#if 0
  SECTION("tree traversal")
  {
    strcpy(R.buffer, "1 2 + 5 7 + *");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    JackAst T(AstNodeType_t::N_TERM, tokens);

    REQUIRE((*tokens)[0].type == TokenType_t::J_INTEGER_CONSTANT);
    REQUIRE((*tokens)[0].value_str == "1");

    // <term>
    auto root_node = T.get_root();

    REQUIRE(root_node->type == AstNodeType_t::N_TERM);

    //         <term>
    //           |
    //  <integer-constant>:1

    auto child = root_node->create_child(AstNodeType_t::N_INTEGER_CONSTANT, (*tokens)[0]);

    REQUIRE(child->type == AstNodeType_t::N_INTEGER_CONSTANT);
    REQUIRE(child->token.value_str == "1");

    auto child_nodes = root_node->get_child_nodes();

    REQUIRE(child_nodes[0] == child);
  }
#endif
}
