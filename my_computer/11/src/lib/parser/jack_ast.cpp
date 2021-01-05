#include "jack_ast.h"

#include <cassert>
//#include <iostream>

using namespace std;

static const JackToken INTERNAL_NODE_TOKEN(TokenType_t::J_INTERNAL,
                                           TokenValue_t::J_UNDEFINED,
                                           "( internal_node )");

//
// Node stuff
//

shared_ptr<AstNode> AstNode::create_child(AstNodeType_t ty, JackToken& tok)
{
  // create the child node and add it to its parent
  auto new_child_node = make_shared<AstNode>(shared_from_this(), ty, tok);

  child_nodes->push_back(move(new_child_node));
  new_child_node = child_nodes->back();

  return new_child_node;
}

//
// AST stuff
//

JackAst::JackAst(AstNodeType_t type, unique_ptr<vector<JackToken>>& tokens)
    : tokens(tokens)
{
  root = make_shared<AstNode>(type);
}

unique_ptr<AstNode> JackAst::parse_term(size_t /*start_index*/)
{
#if 0
  size_t idx = start_index;
  JackToken token = (*tokens)[idx];
  unique_ptr<AstNode> root_node =
      make_unique<AstNode>(INTERNAL_NODE_TOKEN, AstNodeType_t::N_TERM);

  // <integer-constant>
  if (token.type == TokenType_t::J_INTEGER_CONSTANT)
  {
    add_child(root_node, make_unique<AstNode>(token, AstNodeType_t::N_INTEGER_CONSTANT));
  }
  // <string-constant>
  else if (token.type == TokenType_t::J_STRING_CONSTANT)
  {
    auto child_node =
        make_unique<AstNode>(token, AstNodeType_t::N_STRING_CONSTANT);
  }
#if 0
  // <keyword-constant> ::= "true" | "false" | "null" | "this"
  else if (token.type == TokenType_t::J_KEYWORD)
  {
    if ((token.value_enum == TokenValue_t::J_TRUE) ||
        (token.value_enum == TokenValue_t::J_FALSE) ||
        (token.value_enum == TokenValue_t::J_NULL) ||
        (token.value_enum == TokenValue_t::J_THIS))
    {
      *child_node = AstNode(token, AstNodeType_t::N_KEYWORD_CONSTANT);
    }
    else
    {
      assert(0 && "TODO: Put an exception here");
    }
  }
  // <var-name> | <var-name> "[" <expression> "]"
  else if (token.type == TokenType_t::J_IDENTIFIER)
  {
    JackToken next_token = (*tokens)[idx + 1];

    // <var-name> "[" <expression> "]"
    if (next_token.value_enum == TokenValue_t::J_LEFT_BRACKET)
    {
      assert(0);
    }
    // <var-name>
    else
    {
      *child_node = AstNode(token, AstNodeType_t::N_VARIABLE);
    }
  }
#endif

  // return node;
#endif
}

// 1 + 2
// expression
// term op term
// integer-constant:1 + integer-constant:2

unique_ptr<AstNode> JackAst::parse_expression(size_t start_index)
{
  size_t idx = start_index;
  JackToken token = (*tokens)[idx];

  auto expression_p1 = parse_term(idx);

  // const unique_ptr<AstNode>& root_node(t);
  // assert(
  // auto token_vect = make_unique<AstNode>(t);
  // auto root_node = AstNode(t);

  return expression_p1;
}
