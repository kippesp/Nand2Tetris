#include "parse_tree.h"

#include <cassert>
#include <iostream>
#include <string>

using namespace std;

ostream& operator<<(ostream& os, const ParseTreeNode& rhs_ptn)
{
  if (auto rhs_t = dynamic_cast<const ParseTreeTerminal*>(&rhs_ptn))
  {
    os << "(";
    os << ParseTreeNode::to_string(rhs_t->type) << " ";
    os << rhs_t->token.value_str;
    os << ")";
    return os;
  }
  else if (auto rhs_nt = dynamic_cast<const ParseTreeNonTerminal*>(&rhs_ptn))
  {
    os << "(";
    os << ParseTreeNode::to_string(rhs_nt->type) << " ";
    for (auto child_node : rhs_nt->get_child_nodes())
    {
      os << *child_node;
    }
    os << ")";
    return os;
  }

  assert(0 && "Not implemented");
  return os;
}

string ParseTreeNode::to_string(ParseTreeNodeType_t type)
{
  switch (type)
  {
    case ParseTreeNodeType_t::P_UNDEFINED:
      return "P_UNDEFINE";
    case ParseTreeNodeType_t::P_ARRAY_VAR:
      return "P_ARRAY_VAR";
    case ParseTreeNodeType_t::P_COMMA:
      return "P_COMMA";
    case ParseTreeNodeType_t::P_EXPRESSION:
      return "P_EXPRESSION";
    case ParseTreeNodeType_t::P_EXPRESSION_LIST:
      return "P_EXPRESSION_LIST";
    case ParseTreeNodeType_t::P_INTEGER_CONSTANT:
      return "P_INTEGER_CONSTANT";
    case ParseTreeNodeType_t::P_KEYWORD_CONSTANT:
      return "P_KEYWORD_CONSTANT";
    case ParseTreeNodeType_t::P_LEFT_BRACKET:
      return "P_LEFT_BRACKET";
    case ParseTreeNodeType_t::P_LEFT_PARENTHESIS:
      return "P_LEFT_PARENTHESIS";
    case ParseTreeNodeType_t::P_OP:
      return "P_OP";
    case ParseTreeNodeType_t::P_RIGHT_BRACKET:
      return "P_RIGHT_BRACKET";
    case ParseTreeNodeType_t::P_RIGHT_PARENTHESIS:
      return "P_RIGHT_PARENTHESIS";
    case ParseTreeNodeType_t::P_SCALAR_VAR:
      return "P_SCALAR_VAR";
    case ParseTreeNodeType_t::P_STRING_CONSTANT:
      return "P_STRING_CONSTANT";
    case ParseTreeNodeType_t::P_SUBROUTINE_CALL:
      return "P_SUBROUTINE_CALL";
    case ParseTreeNodeType_t::P_SUBROUTINE_NAME:
      return "P_SUBROUTINE_NAME";
    case ParseTreeNodeType_t::P_TERM:
      return "P_TERM";
    case ParseTreeNodeType_t::P_UNARY_OP:
      return "P_UNARY_OP";
  }
}

shared_ptr<ParseTreeNonTerminal> ParseTree::parse_expression()
{
  if (tokens->size() - parse_cursor <= 1)
    return shared_ptr<ParseTreeNonTerminal>{nullptr};

  auto root_node =
      make_shared<ParseTreeNonTerminal>(ParseTreeNodeType_t::P_EXPRESSION);

  auto term1_node = parse_term();

  if (term1_node)
  {
    // <term>
    root_node->create_child(term1_node);

    // {<op> <term>}*
    for (bool done = false; !done; /* empty */)
    {
      done = true;

      if (auto op_node = parse_op())
      {
        auto term2_node = parse_term();

        if (term2_node)
        {
          root_node->create_child(op_node);
          root_node->create_child(term2_node);

          done = false;
        }
      }
    }
  }

  if (root_node->num_child_nodes() == 0)
    return shared_ptr<ParseTreeNonTerminal>{nullptr};

  return root_node;
}

shared_ptr<ParseTreeNonTerminal> ParseTree::parse_expression_list()
{
  auto root_node =
      make_shared<ParseTreeNonTerminal>(ParseTreeNodeType_t::P_EXPRESSION_LIST);

  if (tokens->size() - parse_cursor <= 1) return root_node;

  auto& idx = parse_cursor;

  // {<expression {"," <expression>}*}?
  for (bool done = false; !done; /* empty */)
  {
    done = true;

    if (auto expression_node = parse_expression())
    {
      root_node->create_child(expression_node);
      if (JackToken token = (*tokens)[idx];
          token.value_enum == TokenValue_t::J_COMMA)
      {
        idx++;
        root_node->create_child(ParseTreeNodeType_t::P_COMMA, token);
        done = false;
      }
    }
  }

  return root_node;
}

shared_ptr<ParseTreeNonTerminal> ParseTree::parse_term()
{
  if (tokens->size() - parse_cursor <= 1)
    return shared_ptr<ParseTreeNonTerminal>{nullptr};

  auto& idx = parse_cursor;
  JackToken token = (*tokens)[idx];

  auto root_node =
      make_shared<ParseTreeNonTerminal>(ParseTreeNodeType_t::P_TERM);

  // <integer-constant>
  if (token.type == TokenType_t::J_INTEGER_CONSTANT)
  {
    auto leaf_node =
        root_node->create_child(ParseTreeNodeType_t::P_INTEGER_CONSTANT, token);
  }
  // <string-constant>
  else if (token.type == TokenType_t::J_STRING_CONSTANT)
  {
    root_node->create_child(ParseTreeNodeType_t::P_STRING_CONSTANT, token);
  }
  // <keyword-constant> ::= "true" | "false" | "null" | "this"
  else if (token.type == TokenType_t::J_KEYWORD)
  {
    if ((token.value_enum == TokenValue_t::J_TRUE) ||
        (token.value_enum == TokenValue_t::J_FALSE) ||
        (token.value_enum == TokenValue_t::J_NULL) ||
        (token.value_enum == TokenValue_t::J_THIS))
    {
      root_node->create_child(ParseTreeNodeType_t::P_KEYWORD_CONSTANT, token);
    }
  }
  // <unary-op> <term>
  else if ((token.value_enum == TokenValue_t::J_MINUS) ||
           (token.value_enum == TokenValue_t::J_TILDE))
  {
    idx++;
    auto right_node = parse_term();
    assert(right_node && "Parse error: expecting <term>");

    // add left-node: <unary-op>
    root_node->create_child(ParseTreeNodeType_t::P_UNARY_OP, token);

    // add right-node: <term>
    root_node->create_child(right_node);
  }
  // <subroutine-call>
  else if (auto subroutine_call_node = parse_subroutine_call())
  {
    root_node->create_child(subroutine_call_node);
  }
  // <var-name> | <var-name> "[" <expression> "]"
  else if (token.type == TokenType_t::J_IDENTIFIER)
  {
    if (JackToken lb_token = (*tokens)[idx + 1];
        lb_token.value_enum == TokenValue_t::J_LEFT_BRACKET)
    {
      // array-variable: <var-name> "[" <expression> "]"
      idx++;  // advance over var-name
      idx++;  // advance over "["
      auto expr_node = parse_expression();
      assert(expr_node && "Parse error: expecting <expression>");
      JackToken rb_token = (*tokens)[idx];
      assert((rb_token.value_enum == TokenValue_t::J_RIGHT_BRACKET) &&
             "Parse error: expecting ending \"]\"");

      root_node->create_child(ParseTreeNodeType_t::P_ARRAY_VAR, token);
      root_node->create_child(ParseTreeNodeType_t::P_LEFT_BRACKET, lb_token);
      root_node->create_child(expr_node);
      root_node->create_child(ParseTreeNodeType_t::P_RIGHT_BRACKET, rb_token);
    }
    else
    {
      // scalar-variable: <var-name>
      root_node->create_child(ParseTreeNodeType_t::P_SCALAR_VAR, token);
    }
  }
  // "(" <expression> ")"
  else if (token.value_enum == TokenValue_t::J_LEFT_PARENTHESIS)
  {
    idx++;  // advance over "("
    auto expr_node = parse_expression();
    assert(expr_node && "Parse error: expecting <expression>");
    JackToken rp_token = (*tokens)[idx];
    assert((rp_token.value_enum == TokenValue_t::J_RIGHT_PARENTHESIS) &&
           "Parse error: expecting closing \")\" after expression");

    root_node->create_child(ParseTreeNodeType_t::P_LEFT_PARENTHESIS, token);
    root_node->create_child(expr_node);
    root_node->create_child(ParseTreeNodeType_t::P_RIGHT_PARENTHESIS, rp_token);
  }

  if (root_node->num_child_nodes() == 0)
    return shared_ptr<ParseTreeNonTerminal>{nullptr};

  idx++;
  return root_node;
}

shared_ptr<ParseTreeTerminal> ParseTree::parse_op()
{
  if (tokens->size() - parse_cursor <= 1)
    return shared_ptr<ParseTreeTerminal>{nullptr};

  auto& idx = parse_cursor;
  JackToken token = (*tokens)[idx];

  if (token.type != TokenType_t::J_SYMBOL)
    return shared_ptr<ParseTreeTerminal>{nullptr};

  TokenValue_t value = TokenValue_t::J_UNDEFINED;

  switch (token.value_enum)
  {
    case TokenValue_t::J_PLUS:
    case TokenValue_t::J_MINUS:
    case TokenValue_t::J_ASTERISK:
    case TokenValue_t::J_DIVIDE:
    case TokenValue_t::J_AMPERSAND:
    case TokenValue_t::J_VBAR:
    case TokenValue_t::J_LESS_THAN:
    case TokenValue_t::J_GREATER_THAN:
    case TokenValue_t::J_EQUAL:
      value = token.value_enum;
      break;
    default:
      value = TokenValue_t::J_UNDEFINED;
  }

  if (value == TokenValue_t::J_UNDEFINED)
    return shared_ptr<ParseTreeTerminal>{nullptr};

  idx++;
  return make_shared<ParseTreeTerminal>(ParseTreeNodeType_t::P_OP, token);
}

shared_ptr<ParseTreeNonTerminal> ParseTree::parse_subroutine_call()
{
  // need at least four tokens (including EOF) for the smallest subroutine call
  if (tokens->size() - parse_cursor <= 3)
    return shared_ptr<ParseTreeNonTerminal>{nullptr};

  auto& idx = parse_cursor;

  JackToken first_token = (*tokens)[idx];
  JackToken second_token = (*tokens)[idx + 1];

  auto root_node =
      make_shared<ParseTreeNonTerminal>(ParseTreeNodeType_t::P_SUBROUTINE_CALL);

  if (first_token.type == TokenType_t::J_IDENTIFIER)
  {
    // <subroutine-name> "(" <expression-list> ")"
    if (second_token.value_enum == TokenValue_t::J_LEFT_PARENTHESIS)
    {
      idx++;  // advance over subroutine-name
      idx++;  // advance over "("
      auto expression_list_node = parse_expression_list();
      JackToken rp_token = (*tokens)[idx];
      if (rp_token.value_enum == TokenValue_t::J_RIGHT_PARENTHESIS)
      {
        root_node->create_child(ParseTreeNodeType_t::P_SUBROUTINE_NAME,
                                first_token);
        root_node->create_child(ParseTreeNodeType_t::P_LEFT_PARENTHESIS,
                                second_token);
        if (expression_list_node)
        {
          root_node->create_child(expression_list_node);
        }
        root_node->create_child(ParseTreeNodeType_t::P_RIGHT_PARENTHESIS,
                                rp_token);
      }
      else
      {
        assert(0 && "Parse error: expecting closing \")\" for subroutine call");
      }
    }
    // (<class-name> | <var-name>) "." <subroutine-name> "(" <expression-list>
    // ")"
    else if (second_token.value_enum == TokenValue_t::J_PERIOD)
    {
    }
  }

  if (root_node->num_child_nodes() == 0)
    return shared_ptr<ParseTreeNonTerminal>{nullptr};

  idx++;
  return root_node;
}

shared_ptr<ParseTreeNode> ParseTreeNonTerminal::create_child(
    shared_ptr<ParseTreeNonTerminal> child_node)
{
  child_nodes->push_back(move(child_node));
  auto new_node = child_nodes->back();

  return new_node;
}

shared_ptr<ParseTreeNode> ParseTreeNonTerminal::create_child(
    shared_ptr<ParseTreeTerminal> left_node)
{
  child_nodes->push_back(move(left_node));
  auto new_node = child_nodes->back();

  return new_node;
}

shared_ptr<ParseTreeNode> ParseTreeNonTerminal::create_child(
    ParseTreeNodeType_t ty, JackToken& token)
{
  auto unattached_leaf_node = make_shared<ParseTreeTerminal>(ty, token);
  child_nodes->push_back(move(unattached_leaf_node));
  auto new_node = child_nodes->back();

  return new_node;
}
