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
    case ParseTreeNodeType_t::P_CLASS_OR_VAR_NAME:
      return "P_CLASS_OR_VAR_NAME";
    case ParseTreeNodeType_t::P_DELIMITER:
      return "P_DELIMITER";
    case ParseTreeNodeType_t::P_DO_STATEMENT:
      return "P_DO_STATEMENT";
    case ParseTreeNodeType_t::P_EXPRESSION:
      return "P_EXPRESSION";
    case ParseTreeNodeType_t::P_EXPRESSION_LIST:
      return "P_EXPRESSION_LIST";
    case ParseTreeNodeType_t::P_IF_STATEMENT:
      return "P_IF_STATEMENT";
    case ParseTreeNodeType_t::P_INTEGER_CONSTANT:
      return "P_INTEGER_CONSTANT";
    case ParseTreeNodeType_t::P_KEYWORD:
      return "P_KEYWORD";
    case ParseTreeNodeType_t::P_KEYWORD_CONSTANT:
      return "P_KEYWORD_CONSTANT";
    case ParseTreeNodeType_t::P_LEFT_BRACKET:
      return "P_LEFT_BRACKET";
    case ParseTreeNodeType_t::P_LEFT_PARENTHESIS:
      return "P_LEFT_PARENTHESIS";
    case ParseTreeNodeType_t::P_LET_STATEMENT:
      return "P_LET_STATEMENT";
    case ParseTreeNodeType_t::P_OP:
      return "P_OP";
    case ParseTreeNodeType_t::P_PARAMETER_LIST:
      return "P_PARAMETER_LIST";
    case ParseTreeNodeType_t::P_RETURN_STATEMENT:
      return "P_RETURN_STATEMENT";
    case ParseTreeNodeType_t::P_RIGHT_BRACKET:
      return "P_RIGHT_BRACKET";
    case ParseTreeNodeType_t::P_RIGHT_PARENTHESIS:
      return "P_RIGHT_PARENTHESIS";
    case ParseTreeNodeType_t::P_SCALAR_VAR:
      return "P_SCALAR_VAR";
    case ParseTreeNodeType_t::P_STATEMENT_LIST:
      return "P_STATEMENT_LIST";
    case ParseTreeNodeType_t::P_STRING_CONSTANT:
      return "P_STRING_CONSTANT";
    case ParseTreeNodeType_t::P_SUBROUTINE_BODY:
      return "P_SUBROUTINE_BODY";
    case ParseTreeNodeType_t::P_SUBROUTINE_CALL:
      return "P_SUBROUTINE_CALL";
    case ParseTreeNodeType_t::P_SUBROUTINE_NAME:
      return "P_SUBROUTINE_NAME";
    case ParseTreeNodeType_t::P_TERM:
      return "P_TERM";
    case ParseTreeNodeType_t::P_UNARY_OP:
      return "P_UNARY_OP";
    case ParseTreeNodeType_t::P_VARIABLE_DECL_BLOCK:
      return "P_VARIABLE_DECL_BLOCK";
    case ParseTreeNodeType_t::P_VARIABLE_NAME:
      return "P_VARIABLE_NAME";
    case ParseTreeNodeType_t::P_VARIABLE_TYPE:
      return "P_VARIABLE_TYPE";
    case ParseTreeNodeType_t::P_WHILE_STATEMENT:
      return "P_WHILE_STATEMENT";
  }
}

static bool is_valid_type_token(JackToken& token)
{
  if (token.type == TokenType_t::J_IDENTIFIER)
  {
    return true;
  }
  else if (token.type == TokenType_t::J_KEYWORD)
  {
    switch (token.value_enum)
    {
      case TokenValue_t::J_INT:
      case TokenValue_t::J_CHAR:
      case TokenValue_t::J_BOOLEAN:
        return true;
      default:
        return false;
    }
  }

  return false;
}

shared_ptr<ParseTreeNonTerminal> ParseTree::parse_parameter_list()
{
  if (tokens->size() - parse_cursor <= 2)
    return shared_ptr<ParseTreeNonTerminal>{nullptr};

  auto& idx = parse_cursor;

  auto root_node =
      make_shared<ParseTreeNonTerminal>(ParseTreeNodeType_t::P_PARAMETER_LIST);

  // <parameter-list> ::= {(<type> <var-name>) {"," <type> <var-name>}*}?

  if (JackToken first_type_token = (*tokens)[idx];
      is_valid_type_token(first_type_token))
  {
    idx++;
    JackToken variable_token = (*tokens)[idx];
    idx++;

    assert((variable_token.type == TokenType_t::J_IDENTIFIER) &&
           "Parse error: Invalid variable name");

    root_node->create_child(ParseTreeNodeType_t::P_VARIABLE_TYPE,
                            first_type_token);
    root_node->create_child(ParseTreeNodeType_t::P_VARIABLE_NAME,
                            variable_token);
  }
  else
  {
    return root_node;
  }

  for (bool done = false; !done; /* empty */)
  {
    JackToken delimiter_token = (*tokens)[idx];

    if (delimiter_token.value_enum == TokenValue_t::J_COMMA)
    {
      idx++;
      JackToken type_token = (*tokens)[idx];
      idx++;

      assert(is_valid_type_token(type_token) &&
             "Parse error: expected int|char|boolean|CLASSNAME for parameter "
             "type");

      JackToken variable_token = (*tokens)[idx];
      idx++;

      assert((variable_token.type == TokenType_t::J_IDENTIFIER) &&
             "Parse error: Invalid variable name");

      root_node->create_child(ParseTreeNodeType_t::P_DELIMITER,
                              delimiter_token);
      root_node->create_child(ParseTreeNodeType_t::P_VARIABLE_TYPE, type_token);
      root_node->create_child(ParseTreeNodeType_t::P_VARIABLE_NAME,
                              variable_token);
    }
    else
    {
      done = true;
    }
  }

  return root_node;
}

shared_ptr<ParseTreeNonTerminal> ParseTree::parse_subroutine_body()
{
  if (tokens->size() - parse_cursor <= 2)
    return shared_ptr<ParseTreeNonTerminal>{nullptr};

  auto& idx = parse_cursor;
  JackToken lb_token = (*tokens)[idx];
  idx++;

  if (lb_token.value_enum != TokenValue_t::J_LEFT_BRACE)
    return shared_ptr<ParseTreeNonTerminal>{nullptr};

  auto root_node =
      make_shared<ParseTreeNonTerminal>(ParseTreeNodeType_t::P_SUBROUTINE_BODY);

  // <subroutine-body> ::= "{" {<var-decl>}* <statements> "}"

  root_node->create_child(ParseTreeNodeType_t::P_DELIMITER, lb_token);

  if (auto variable_decl_block = parse_variable_decl_block())
    root_node->create_child(variable_decl_block);

  auto statements_node = parse_statements();
  assert(statements_node && "<statements> must not be null");

  root_node->create_child(statements_node);

  JackToken rb_token = (*tokens)[idx];
  idx++;

  if (rb_token.value_enum != TokenValue_t::J_RIGHT_BRACE)
  {
    assert(0 && "Parse error: expecting <right-brace> after <statement-list>");
  }

  root_node->create_child(ParseTreeNodeType_t::P_DELIMITER, rb_token);

  return root_node;
}

shared_ptr<ParseTreeNonTerminal> ParseTree::parse_variable_decl_block()
{
  if (tokens->size() - parse_cursor <= 4)
    return shared_ptr<ParseTreeNonTerminal>{nullptr};

  auto& idx = parse_cursor;
  JackToken keyword_token = (*tokens)[idx];

  if (keyword_token.value_enum != TokenValue_t::J_VAR)
    return shared_ptr<ParseTreeNonTerminal>{nullptr};

  auto root_node = make_shared<ParseTreeNonTerminal>(
      ParseTreeNodeType_t::P_VARIABLE_DECL_BLOCK);

  idx++;

  // <var-decl> ::= "var" <type> <var-name> {"," <var-name>}* ";"
  JackToken type_token = (*tokens)[idx];
  idx++;

  root_node->create_child(ParseTreeNodeType_t::P_KEYWORD, keyword_token);

  if (type_token.type == TokenType_t::J_IDENTIFIER)
  {
    root_node->create_child(ParseTreeNodeType_t::P_VARIABLE_TYPE, type_token);
  }
  else if (type_token.type == TokenType_t::J_KEYWORD)
  {
    switch (type_token.value_enum)
    {
      case TokenValue_t::J_INT:
      case TokenValue_t::J_CHAR:
      case TokenValue_t::J_BOOLEAN:
        root_node->create_child(ParseTreeNodeType_t::P_VARIABLE_TYPE,
                                type_token);
        break;
      default:
        assert(0 &&
               "Parse error: expected int|char|boolean after <var> keyword");
    }
  }
  else
  {
    assert(0 && "Parse error: expected <class_type> after <var> keyword");
  }

  JackToken first_variable_token = (*tokens)[idx];
  idx++;

  assert((first_variable_token.type == TokenType_t::J_IDENTIFIER) &&
         "Parse error: expected variable name after type name");

  root_node->create_child(ParseTreeNodeType_t::P_VARIABLE_NAME,
                          first_variable_token);

  for (bool done = false; !done; /* empty */)
  {
    JackToken delimiter_token = (*tokens)[idx];

    if (delimiter_token.value_enum == TokenValue_t::J_COMMA)
    {
      idx++;
      JackToken variable_token = (*tokens)[idx];
      idx++;

      assert((variable_token.type == TokenType_t::J_IDENTIFIER) &&
             "Parse error: Invalid variable name");

      root_node->create_child(ParseTreeNodeType_t::P_DELIMITER,
                              delimiter_token);
      root_node->create_child(ParseTreeNodeType_t::P_VARIABLE_NAME,
                              variable_token);
    }
    else
    {
      done = true;
    }
  }

  JackToken final_token = (*tokens)[idx];
  idx++;

  assert((final_token.value_enum == TokenValue_t::J_SEMICOLON) &&
         "Parse error: expected <semicolon> at end of variable declaration");

  root_node->create_child(ParseTreeNodeType_t::P_DELIMITER, final_token);

  return root_node;
}

shared_ptr<ParseTreeNonTerminal> ParseTree::parse_statements()
{
  auto root_node =
      make_shared<ParseTreeNonTerminal>(ParseTreeNodeType_t::P_STATEMENT_LIST);

  // <statements> ::= {<statement>}*
  for (bool done = false; !done; /* empty */)
  {
    if (auto statement_node = parse_statement())
      root_node->create_child(statement_node);
    else
      done = true;
  }

  return root_node;
}

shared_ptr<ParseTreeNonTerminal> ParseTree::parse_statement()
{
  if (tokens->size() - parse_cursor <= 2)
    return shared_ptr<ParseTreeNonTerminal>{nullptr};

  auto& idx = parse_cursor;
  JackToken keyword_token = (*tokens)[idx];

  if (keyword_token.type == TokenType_t::J_KEYWORD)
  {
    using parsefn_t = shared_ptr<ParseTreeNonTerminal> (ParseTree::*)();
    parsefn_t parser_fn = &ParseTree::parse_return_statement;

    switch (keyword_token.value_enum)
    {
      case TokenValue_t::J_LET:
        parser_fn = &ParseTree::parse_let_statement;
        break;
      case TokenValue_t::J_IF:
        parser_fn = &ParseTree::parse_if_statement;
        break;
      case TokenValue_t::J_WHILE:
        parser_fn = &ParseTree::parse_while_statement;
        break;
      case TokenValue_t::J_DO:
        parser_fn = &ParseTree::parse_do_statement;
        break;
      case TokenValue_t::J_RETURN:
        parser_fn = &ParseTree::parse_return_statement;
        break;
      default:
        assert(0 && "Unhandled token case");
    }

    if (auto statement_node = (this->*parser_fn)())
      return statement_node;
    else
      assert(0 && "Parse error: failed while parsing <statement>");
  }

  return shared_ptr<ParseTreeNonTerminal>{nullptr};
}

shared_ptr<ParseTreeNonTerminal> ParseTree::parse_let_statement()
{
  if (tokens->size() - parse_cursor <= 5)
    return shared_ptr<ParseTreeNonTerminal>{nullptr};

  auto& idx = parse_cursor;
  JackToken keyword_token = (*tokens)[idx];
  idx++;

  assert(keyword_token.type == TokenType_t::J_KEYWORD);
  assert(keyword_token.value_enum == TokenValue_t::J_LET);

  auto root_node =
      make_shared<ParseTreeNonTerminal>(ParseTreeNodeType_t::P_LET_STATEMENT);

  // <let-statement> ::= "let" <var-name> {"[" <expression> "]"}? "="
  // <expression> ";"
  JackToken varname = (*tokens)[idx];
  idx++;

  if (varname.type != TokenType_t::J_IDENTIFIER)
    assert(0 && "Parse error: expecting <var-name> after 'let' keyword");
  JackToken next_token = (*tokens)[idx];
  idx++;

  if (next_token.value_enum == TokenValue_t::J_EQUAL)
  {
    auto expression_node = parse_expression();

    if (!expression_node)
      assert(0 && "Parse error: expecting <expression> after '='");

    if ((*tokens)[idx].value_enum != TokenValue_t::J_SEMICOLON)
      assert(0 && "Parse error: expecting <semicolon> after <expression>");

    root_node->create_child(ParseTreeNodeType_t::P_KEYWORD, keyword_token);
    root_node->create_child(ParseTreeNodeType_t::P_SCALAR_VAR, varname);
    root_node->create_child(ParseTreeNodeType_t::P_OP, next_token);
    root_node->create_child(expression_node);
    root_node->create_child(ParseTreeNodeType_t::P_DELIMITER, (*tokens)[idx]);
    idx++;
  }
  else if (next_token.value_enum == TokenValue_t::J_LEFT_BRACKET)
  {
    auto index_expression_node = parse_expression();
    JackToken rb_token = (*tokens)[idx];
    idx++;

    if (rb_token.value_enum != TokenValue_t::J_RIGHT_BRACKET)
      assert(0 && "Parse error: expecting <right_bracket> after <expression>");

    JackToken equal_token = (*tokens)[idx];
    idx++;

    if (equal_token.value_enum != TokenValue_t::J_EQUAL)
      assert(0 && "Parse error: expecting <equal> after ']'");

    auto expression_node = parse_expression();

    if (!expression_node)
      assert(0 && "Parse error: expecting <expression> after '='");

    if ((*tokens)[idx].value_enum != TokenValue_t::J_SEMICOLON)
      assert(0 && "Parse error: expecting <semicolon> after <expression>");

    root_node->create_child(ParseTreeNodeType_t::P_KEYWORD, keyword_token);
    root_node->create_child(ParseTreeNodeType_t::P_ARRAY_VAR, varname);
    root_node->create_child(ParseTreeNodeType_t::P_LEFT_BRACKET, next_token);
    root_node->create_child(index_expression_node);
    root_node->create_child(ParseTreeNodeType_t::P_RIGHT_BRACKET, rb_token);
    root_node->create_child(ParseTreeNodeType_t::P_OP, equal_token);
    root_node->create_child(expression_node);
    root_node->create_child(ParseTreeNodeType_t::P_DELIMITER, (*tokens)[idx]);
    idx++;
  }
  else
  {
    assert(0 && "Parse error: malformed let statement");
  }

  return root_node;
}

shared_ptr<ParseTreeNonTerminal> ParseTree::parse_if_statement()
{
  if (tokens->size() - parse_cursor <= 6)
    return shared_ptr<ParseTreeNonTerminal>{nullptr};

  auto& idx = parse_cursor;
  JackToken keyword_token = (*tokens)[idx];
  idx++;

  assert(keyword_token.type == TokenType_t::J_KEYWORD);
  assert(keyword_token.value_enum == TokenValue_t::J_IF);

  auto root_node =
      make_shared<ParseTreeNonTerminal>(ParseTreeNodeType_t::P_IF_STATEMENT);

  //
  // "if" "(" <expression ")" "{" <statements> "}"
  //

  JackToken lp_token = (*tokens)[idx];
  idx++;

  if (lp_token.value_enum != TokenValue_t::J_LEFT_PARENTHESIS)
    assert(0 && "Parse error: expecting <left_parenthesis> after 'if' keyword");

  if (auto expression_node = parse_expression())
  {
    JackToken rp_token = (*tokens)[idx];
    idx++;

    if (rp_token.value_enum != TokenValue_t::J_RIGHT_PARENTHESIS)
      assert(0 &&
             "Parse error: expecting <right_parenthesis> after <expression>");

    JackToken lb_token = (*tokens)[idx];
    idx++;

    if (lb_token.value_enum != TokenValue_t::J_LEFT_BRACE)
      assert(0 && "Parse error: expecting <left_brace> after ')'");

    auto statements_node = parse_statements();
    assert(statements_node && "<statements> must not be null");

    JackToken rb_token = (*tokens)[idx];
    idx++;

    if (rb_token.value_enum != TokenValue_t::J_RIGHT_BRACE)
      assert(0 &&
             "Parse error: expecting <right_brace> after TRUE <statements>");

    root_node->create_child(ParseTreeNodeType_t::P_KEYWORD, keyword_token);
    root_node->create_child(ParseTreeNodeType_t::P_DELIMITER, lp_token);
    root_node->create_child(expression_node);
    root_node->create_child(ParseTreeNodeType_t::P_DELIMITER, rp_token);
    root_node->create_child(ParseTreeNodeType_t::P_DELIMITER, lb_token);
    root_node->create_child(statements_node);
    root_node->create_child(ParseTreeNodeType_t::P_DELIMITER, rb_token);
  }
  else
  {
    assert(0 &&
           "Parse error: expecting <expression> in 'if' statement true branch");
  }

  //
  // {"else" "{" <statements> "}"}?
  //

  if (JackToken else_token = (*tokens)[idx];
      (root_node->num_child_nodes() > 0) &&
      (else_token.value_enum == TokenValue_t::J_ELSE))
  {
    idx++;

    JackToken lb_token = (*tokens)[idx];
    idx++;

    if (lb_token.value_enum != TokenValue_t::J_LEFT_BRACE)
      assert(0 && "Parse error: expecting <left_brace> after 'else'");

    auto statements_node = parse_statements();

    assert(statements_node && "<statements> must not be null");

    JackToken rb_token = (*tokens)[idx];
    idx++;

    if (rb_token.value_enum != TokenValue_t::J_RIGHT_BRACE)
      assert(0 &&
             "Parse error: expecting <right_brace> after FALSE <statements>");

    root_node->create_child(ParseTreeNodeType_t::P_KEYWORD, else_token);
    root_node->create_child(ParseTreeNodeType_t::P_DELIMITER, lb_token);
    root_node->create_child(statements_node);
    root_node->create_child(ParseTreeNodeType_t::P_DELIMITER, rb_token);
  }

  return root_node;
}

shared_ptr<ParseTreeNonTerminal> ParseTree::parse_while_statement()
{
  if (tokens->size() - parse_cursor <= 6)
    return shared_ptr<ParseTreeNonTerminal>{nullptr};

  auto& idx = parse_cursor;
  JackToken keyword_token = (*tokens)[idx];

  assert(keyword_token.type == TokenType_t::J_KEYWORD);
  assert(keyword_token.value_enum == TokenValue_t::J_WHILE);

  auto root_node =
      make_shared<ParseTreeNonTerminal>(ParseTreeNodeType_t::P_WHILE_STATEMENT);

  // <while-statement> ::= "while" "(" <expression> ")" "{" <statements> "}"
  idx++;
  JackToken lp_token = (*tokens)[idx];

  if (lp_token.value_enum != TokenValue_t::J_LEFT_PARENTHESIS)
    assert(0 &&
           "Parse error: expecting <left_parenthesis> after 'while' keyword");

  idx++;

  if (auto expression_node = parse_expression())
  {
    JackToken rp_token = (*tokens)[idx];
    if (rp_token.value_enum != TokenValue_t::J_RIGHT_PARENTHESIS)
      assert(0 &&
             "Parse error: expecting <right_parenthesis> after <expression>");

    idx++;
    JackToken lb_token = (*tokens)[idx];

    if (lb_token.value_enum != TokenValue_t::J_LEFT_BRACE)
      assert(0 && "Parse error: expecting <left_brace> after ')'");

    auto statements_node = parse_statements();

    assert(statements_node && "<statements> must not be null");

    idx++;
    JackToken rb_token = (*tokens)[idx];

    if (rb_token.value_enum != TokenValue_t::J_RIGHT_BRACE)
      assert(0 && "Parse error: expecting <right_brace> after <statements>");

    root_node->create_child(ParseTreeNodeType_t::P_KEYWORD, keyword_token);
    root_node->create_child(ParseTreeNodeType_t::P_DELIMITER, lp_token);
    root_node->create_child(expression_node);
    root_node->create_child(ParseTreeNodeType_t::P_DELIMITER, rp_token);
    root_node->create_child(ParseTreeNodeType_t::P_DELIMITER, lb_token);
    root_node->create_child(statements_node);
    root_node->create_child(ParseTreeNodeType_t::P_DELIMITER, rb_token);
    idx++;
  }
  else
  {
    assert(0 && "Parse error: expecting <expression> in 'while' statement");
  }

  return root_node;
}

shared_ptr<ParseTreeNonTerminal> ParseTree::parse_do_statement()
{
  assert(tokens->size() - parse_cursor > 2);

  auto& idx = parse_cursor;
  JackToken token = (*tokens)[idx];
  JackToken token_second = (*tokens)[idx + 1];

  assert(token.type == TokenType_t::J_KEYWORD);
  assert(token.value_enum == TokenValue_t::J_DO);

  auto root_node =
      make_shared<ParseTreeNonTerminal>(ParseTreeNodeType_t::P_DO_STATEMENT);

  // <do-statement> ::= "do" <subroutine-call> ";"
  idx++;

  if (auto subroutine_call_node = parse_subroutine_call())
  {
    if ((*tokens)[idx].value_enum != TokenValue_t::J_SEMICOLON)
    {
      assert(0 && "Parse error: expecting <semicolon> after <subroutine-call>");
    }

    root_node->create_child(ParseTreeNodeType_t::P_KEYWORD, token);
    root_node->create_child(subroutine_call_node);
    root_node->create_child(ParseTreeNodeType_t::P_DELIMITER, (*tokens)[idx]);

    idx++;
  }

  return root_node;
}

shared_ptr<ParseTreeNonTerminal> ParseTree::parse_return_statement()
{
  assert(tokens->size() - parse_cursor > 2);

  auto& idx = parse_cursor;
  JackToken token = (*tokens)[idx];
  JackToken token_second = (*tokens)[idx + 1];

  assert(token.type == TokenType_t::J_KEYWORD);
  assert(token.value_enum == TokenValue_t::J_RETURN);

  auto root_node = make_shared<ParseTreeNonTerminal>(
      ParseTreeNodeType_t::P_RETURN_STATEMENT);

  // <return-statement> ::= "return" {<expression>}? ";"
  if (token_second.value_enum == TokenValue_t::J_SEMICOLON)
  {
    root_node->create_child(ParseTreeNodeType_t::P_KEYWORD, token);
    root_node->create_child(ParseTreeNodeType_t::P_DELIMITER, token_second);
    idx++;
    idx++;
  }
  else
  {
    idx++;
    auto expression_node = parse_expression();

    if (expression_node)
    {
      if ((*tokens)[idx].value_enum != TokenValue_t::J_SEMICOLON)
      {
        assert(0 && "Parse error: expecting <semicolon> after <expression>");
      }

      root_node->create_child(ParseTreeNodeType_t::P_KEYWORD, token);
      root_node->create_child(expression_node);
      root_node->create_child(ParseTreeNodeType_t::P_DELIMITER, (*tokens)[idx]);

      idx++;
    }
  }

  return root_node;
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
        root_node->create_child(ParseTreeNodeType_t::P_DELIMITER, token);
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
    idx++;
  }
  // <string-constant>
  else if (token.type == TokenType_t::J_STRING_CONSTANT)
  {
    root_node->create_child(ParseTreeNodeType_t::P_STRING_CONSTANT, token);
    idx++;
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
      idx++;
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
    idx++;
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

    idx++;
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
    idx++;
  }

  if (root_node->num_child_nodes() == 0)
    return shared_ptr<ParseTreeNonTerminal>{nullptr};

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
  // need at least four tokens (including EOF) for the smaller subroutine call
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

      assert((rp_token.value_enum == TokenValue_t::J_RIGHT_PARENTHESIS) &&
             "Parse error: expecting closing \")\" for subroutine call");

      root_node->create_child(ParseTreeNodeType_t::P_SUBROUTINE_NAME,
                              first_token);
      root_node->create_child(ParseTreeNodeType_t::P_LEFT_PARENTHESIS,
                              second_token);
      root_node->create_child(expression_list_node);
      root_node->create_child(ParseTreeNodeType_t::P_RIGHT_PARENTHESIS,
                              rp_token);
    }
    // (<class-name> | <var-name>) "."
    //    <subroutine-name> "(" <expression-list> ")"
    else if (second_token.value_enum == TokenValue_t::J_PERIOD)
    {
      // need at least six tokens (including EOF) for the larger subroutine call
      if (tokens->size() - parse_cursor <= 5)
        return shared_ptr<ParseTreeNonTerminal>{nullptr};

      JackToken subroutine_token = (*tokens)[idx + 2];
      JackToken lp_token = (*tokens)[idx + 3];

      if ((subroutine_token.type == TokenType_t::J_IDENTIFIER) &&
          (lp_token.value_enum == TokenValue_t::J_LEFT_PARENTHESIS))
      {
        idx++;  // advance over <class-name> | <var-name>
        idx++;  // advance over "."
        idx++;  // advance over subroutine-name
        idx++;  // advance over "("
        auto expression_list_node = parse_expression_list();
        JackToken rp_token = (*tokens)[idx];

        assert((rp_token.value_enum == TokenValue_t::J_RIGHT_PARENTHESIS) &&
               "Parse error: expecting closing \")\" for subroutine call");

        root_node->create_child(ParseTreeNodeType_t::P_CLASS_OR_VAR_NAME,
                                first_token);
        root_node->create_child(ParseTreeNodeType_t::P_DELIMITER, second_token);
        root_node->create_child(ParseTreeNodeType_t::P_SUBROUTINE_NAME,
                                subroutine_token);
        root_node->create_child(ParseTreeNodeType_t::P_LEFT_PARENTHESIS,
                                lp_token);
        root_node->create_child(expression_list_node);
        root_node->create_child(ParseTreeNodeType_t::P_RIGHT_PARENTHESIS,
                                rp_token);
      }
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
