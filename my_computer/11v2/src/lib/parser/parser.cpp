#include "parser.h"
#include "parser/ast.h"
#include "tokenizer/jack_token.h"

#include <cassert>
#include <functional>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_set>

namespace jfcl {

Parser::Parser(Tokens_t& tokens, AstTree& ast)
    : AST(ast),
      token_iter(tokens.begin()),
      token_iter_end(tokens.end()),
      current_token(*token_iter),
      peek_token(*(++token_iter))
{
}

void Parser::require_token(TokenValue_t token, TokenValue_t token_value)
{
  if (current_token.get().value_enum != token_value)
  {
    std::stringstream ss;

    ss << "Line #" << current_token.get().line_number << ": "
       << "Expected " << JackToken::to_string(token_value) << " while parsing "
       << JackToken::to_string(token);

    fatal_error(ss.str());
  }
}

void Parser::require_token(JackTokenCRef token, TokenValue_t token_value)
{
  require_token(token.get().value_enum, token_value);
}

AstNodeRef Parser::create_ast_node(AstNodeType_t type)
{
  return AST.add(AstNode(type, current_token.get().line_number));
}

AstNodeRef Parser::parse_class(std::string& class_name)
{
  const auto start_token = TokenValue_t::J_CLASS;
  assert(current_token.get().value_enum == start_token);

  AstNodeRef ClassAst = create_ast_node(AstNodeType_t::N_CLASS_DECL);

  get_next_token();
  require_token(current_token, TokenValue_t::J_IDENTIFIER);

  ClassAst.get().value = current_token.get().value_str;
  class_name = current_token.get().value_str;

  get_next_token();
  require_token(current_token, TokenValue_t::J_LEFT_BRACE);
  get_next_token();

  // <class-var-decl>
  //      ::= ("static" | "field") <type> <var-name> {"," <var-name>}* ";"

  if ((current_token.get().value_enum == TokenValue_t::J_STATIC) ||
      (current_token.get().value_enum == TokenValue_t::J_FIELD))
  {
    AstNodeRef const class_vars_block_node = parse_classvar_decl_block();

    ClassAst.get().add_child(class_vars_block_node);
  }

  while (current_token.get().value_enum != TokenValue_t::J_RIGHT_BRACE)
  {
    AstNodeRef const SubroutineAstNodeRef = parse_subroutine();
    ClassAst.get().add_child(SubroutineAstNodeRef);
  }

  require_token(current_token, TokenValue_t::J_RIGHT_BRACE);

  return ClassAst;
}

AstNodeRef Parser::parse_classvar_decl_block()
{
  AstNodeRef class_vars_block_node =
      create_ast_node(AstNodeType_t::N_CLASS_VARIABLES);

  while ((current_token.get().value_enum == TokenValue_t::J_STATIC) ||
         (current_token.get().value_enum == TokenValue_t::J_FIELD))
  {
    std::string var_scope;

    if (current_token.get().value_enum == TokenValue_t::J_STATIC)
    {
      var_scope = "static";
    }
    else if (current_token.get().value_enum == TokenValue_t::J_FIELD)
    {
      var_scope = "field";
    }
    else
    {
      std::stringstream ss;

      ss << "Line #" << current_token.get().line_number << ": "
         << "Expected static|field while parsing "
         << JackToken::to_string(current_token.get().value_enum);

      fatal_error(ss.str());
    }

    get_next_token();

    AstNodeRef var_type_node = parse_type(AstNodeType_t::N_VARIABLE_TYPE);

    if (var_type_node.get() == AST.get_empty_node_ref().get())
    {
      std::stringstream ss;

      ss << "Line #" << current_token.get().line_number << ": "
         << "Expected int|char|boolean|ClassName while parsing "
         << JackToken::to_string(current_token.get().value_enum);

      fatal_error(ss.str());
    }

    auto add_def_to_block = [&]() {
      require_token(current_token, TokenValue_t::J_IDENTIFIER);

      AstNodeRef const class_var_decl = create_ast_node(
          AstNodeType_t::N_VARIABLE_DECL, current_token.get().value_str);

      class_var_decl.get().add_child(
          create_ast_node(AstNodeType_t::N_CLASS_VARIABLE_SCOPE, var_scope));

      class_var_decl.get().add_child(var_type_node);

      class_vars_block_node.get().add_child(class_var_decl);
      get_next_token();
    };

    require_token(current_token, TokenValue_t::J_IDENTIFIER);
    add_def_to_block();

    while (current_token.get().value_enum == TokenValue_t::J_COMMA)
    {
      get_next_token();
      require_token(current_token, TokenValue_t::J_IDENTIFIER);
      add_def_to_block();
    }

    require_token(current_token, TokenValue_t::J_SEMICOLON);
    get_next_token();
  }

  return class_vars_block_node;
}

AstNodeRef Parser::parse_subroutine()
{
  AstNodeRef SubrDeclAst = AST.get_empty_node_ref();

  if (current_token.get().value_enum == TokenValue_t::J_FUNCTION)
  {
    SubrDeclAst = create_ast_node(AstNodeType_t::N_FUNCTION_DECL);
  }
  else if (current_token.get().value_enum == TokenValue_t::J_METHOD)
  {
    SubrDeclAst = create_ast_node(AstNodeType_t::N_METHOD_DECL);
  }
  else if (current_token.get().value_enum == TokenValue_t::J_CONSTRUCTOR)
  {
    SubrDeclAst = create_ast_node(AstNodeType_t::N_CONSTRUCTOR_DECL);
  }
  else
  {
    std::stringstream ss;

    ss << "Line #" << current_token.get().line_number << ": "
       << "Expected " << JackToken::to_string(TokenValue_t::J_FUNCTION) << "|"
       << JackToken::to_string(TokenValue_t::J_METHOD) << "|"
       << JackToken::to_string(TokenValue_t::J_CONSTRUCTOR) << " while parsing "
       << JackToken::to_string(TokenValue_t::J_CLASS);
    fatal_error(ss.str());
  }

  get_next_token();

  AstNodeRef const SubrDescrAst =
      create_ast_node(AstNodeType_t::N_SUBROUTINE_DESCR);

  // <subroutine-decl>  ::= ("constructor" | "function" | "method")
  //                        ("void" | <type>) <subroutine-name>
  //                        \---------------/
  //                                /
  //                               /
  //                     SubrDeclReturnTypeAst

  AstNodeRef SubrDeclReturnTypeAst = AST.get_empty_node_ref();

  SubrDeclReturnTypeAst = parse_type(AstNodeType_t::N_RETURN_TYPE);

  if (SubrDeclReturnTypeAst.get() == AST.get_empty_node_ref().get())
  {
    std::stringstream ss;

    ss << "Line #" << current_token.get().line_number << ": "
       << "Expected int|char|boolean|void|ClassName while parsing "
       << JackToken::to_string(current_token.get().value_enum);

    fatal_error(ss.str());
  }

  SubrDescrAst.get().add_child(SubrDeclReturnTypeAst);
  SubrDeclAst.get().add_child(SubrDescrAst);

  // <subroutine-decl>  ::= ("constructor" | "function" | "method")
  //                        ("void" | <type>) <subroutine-name>
  //                                          \---------------/
  //                                                 /
  //                                                /
  //                                         SubrDeclAst

  require_token(current_token, TokenValue_t::J_IDENTIFIER);

  SubrDeclAst.get().value = current_token.get().value_str;

  get_next_token();
  require_token(current_token, TokenValue_t::J_LEFT_PARENTHESIS);
  get_next_token();

  // <subroutine-decl>  ::= ("constructor" | "function" | "method")
  //                        ("void" | <type>) <subroutine-name>
  //                        "(" <input-parameters> ")" <subroutine-body>
  //                            \----------------/
  //                                    /
  //                                   /
  //          +-----------------------+
  //         /                           +----- N_VARIABLE_DECL
  //        /                           /
  // /----------------\        /-----------------/
  // <input-parameters>   ::= {(<type> <var-name>) {"," <type> <var-name>}*}?
  //       |
  //       +-- N_INPUT_PARAMETERS

  if (current_token.get().value_enum != TokenValue_t::J_RIGHT_PARENTHESIS)
  {
    AstNodeRef const input_parms_root =
        create_ast_node(AstNodeType_t::N_INPUT_PARAMETERS);

    while (current_token.get().value_enum != TokenValue_t::J_RIGHT_PARENTHESIS)
    {
      AstNodeRef const parm_type_node =
          parse_type(AstNodeType_t::N_VARIABLE_TYPE);

      if (parm_type_node.get() == AST.get_empty_node_ref().get())
      {
        std::stringstream ss;

        ss << "Line #" << current_token.get().line_number << ": "
           << "Expected int|char|boolean|ClassName while parsing "
           << JackToken::to_string(current_token.get().value_enum);

        fatal_error(ss.str());
      }

      require_token(current_token, TokenValue_t::J_IDENTIFIER);

      AstNodeRef const variable_decl_root = create_ast_node(
          AstNodeType_t::N_VARIABLE_DECL, current_token.get().value_str);

      variable_decl_root.get().add_child(parm_type_node);
      input_parms_root.get().add_child(variable_decl_root);

      get_next_token();

      if (current_token.get().value_enum == TokenValue_t::J_COMMA)
      {
        get_next_token();
      }
    }

    SubrDescrAst.get().add_child(input_parms_root);
  }

  require_token(current_token, TokenValue_t::J_RIGHT_PARENTHESIS);
  get_next_token();

  require_token(current_token, TokenValue_t::J_LEFT_BRACE);
  get_next_token();

  //
  // <subroutine-body>  ::= "{" {<var-decl>}* {<statement>}* "}"
  //                            \----------/
  //                                   |
  //                                   +------- N_LOCAL_VARIABLES
  //
  //                                        ------ N_VARIABLE_DECL
  //                                       /
  //                              /---------------/
  // <var-decl>         ::= "var" <type> <var-name> {"," <var-name>}* ";"
  //                              \----/
  //                                /
  //            N_VARIABLE_TYPE ----

  AstNodeRef const SubroutineBodyAst =
      create_ast_node(AstNodeType_t::N_SUBROUTINE_BODY);

  if (current_token.get().value_enum == TokenValue_t::J_VAR)
  {
    AstNodeRef const SubroutineVarDeclBlockAst =
        create_ast_node(AstNodeType_t::N_LOCAL_VARIABLES);

    while (current_token.get().value_enum == TokenValue_t::J_VAR)
    {
      get_next_token();
      AstNodeRef const var_type = parse_type(AstNodeType_t::N_VARIABLE_TYPE);

      if (var_type.get() == AST.get_empty_node_ref().get())
      {
        std::stringstream ss;

        ss << "Line #" << current_token.get().line_number << ": "
           << "Expected int|char|boolean|ClassName while parsing "
           << JackToken::to_string(current_token.get().value_enum);

        fatal_error(ss.str());
      }

      for (bool done = false; !done; /* empty */)
      {
        AstNodeRef const SubroutineVarDeclAst = create_ast_node(
            AstNodeType_t::N_VARIABLE_DECL, current_token.get().value_str);
        get_next_token();

        SubroutineVarDeclAst.get().add_child(var_type);

        SubroutineVarDeclBlockAst.get().add_child(SubroutineVarDeclAst);

        if (current_token.get().value_enum == TokenValue_t::J_COMMA)
        {
          get_next_token();
        }
        else
        {
          done = true;
        }
      }

      require_token(current_token, TokenValue_t::J_SEMICOLON);
      get_next_token();
    }

    SubrDescrAst.get().add_child(SubroutineVarDeclBlockAst);
  }

  //
  // <subroutine-body>  ::= "{" {<var-decl>}* {<statement>}* "}"
  //                                           \---------/
  //                                                /
  //                                               /
  //                                      SubroutineBodyAst

  if (peek_token.get().value_enum != TokenValue_t::J_RIGHT_BRACE)
  {
    AstNodeRef const StatementBlockAst =
        create_ast_node(AstNodeType_t::N_STATEMENT_BLOCK);

    while (current_token.get().value_enum != TokenValue_t::J_RIGHT_BRACE)
    {
      AstNodeRef const StatementAst = parse_statement();
      if (StatementAst.get() == AST.get_empty_node_ref().get())
      {
        std::stringstream ss;

        ss << "Line #" << current_token.get().line_number << ": "
           << "Expected statement start token while parsing "
           << JackToken::to_string(current_token.get().value_enum);

        fatal_error(ss.str());
      }
      assert((StatementAst.get() != AST.get_empty_node_ref().get()) &&
             "statement required");
      StatementBlockAst.get().add_child(StatementAst);
    }

    SubroutineBodyAst.get().add_child(StatementBlockAst);
  }

  SubrDeclAst.get().add_child(SubroutineBodyAst);

  require_token(current_token, TokenValue_t::J_RIGHT_BRACE);
  get_next_token();

  return SubrDeclAst;
}

// <statement-block>  ::= "{" {<statement>}* "}"
AstNodeRef Parser::parse_inner_statements()
{
  AstNodeRef statement_block_root =
      create_ast_node(AstNodeType_t::N_STATEMENT_BLOCK);

  for (bool done = false; !done; /* empty */)
  {
    AstNodeRef const statement_ast = parse_statement();

    if (statement_ast.get() != AST.get_empty_node_ref().get())
    {
      statement_block_root.get().add_child(statement_ast);
    }
    else
    {
      done = true;
    }
  }

  return statement_block_root;
}

AstNodeRef Parser::parse_statement()
{
  std::optional<AstNodeRef> StatementAst;

  if (current_token.get().value_enum == TokenValue_t::J_LET)
  {
    StatementAst = parse_let_statement();
  }
  else if (current_token.get().value_enum == TokenValue_t::J_DO)
  {
    StatementAst = parse_do_statement();
  }
  else if (current_token.get().value_enum == TokenValue_t::J_RETURN)
  {
    StatementAst = parse_return_statement();
  }
  else if (current_token.get().value_enum == TokenValue_t::J_WHILE)
  {
    StatementAst = parse_while_statement();
  }
  else if (current_token.get().value_enum == TokenValue_t::J_IF)
  {
    StatementAst = parse_if_statement();
  }

  if (StatementAst.has_value())
  {
    return StatementAst.value();
  }
  else
  {
    return AST.get_empty_node_ref();
  }
}

AstNodeRef Parser::parse_let_statement()
{
  const auto start_token = TokenValue_t::J_LET;
  assert(current_token.get().value_enum == start_token);

  AstNodeRef LetAst = create_ast_node(AstNodeType_t::N_LET_STATEMENT);
  get_next_token();

  require_token(current_token, TokenValue_t::J_IDENTIFIER);

  LetAst.get().add_child(parse_variable());

  require_token(current_token, TokenValue_t::J_EQUAL);
  get_next_token();

  AstNode& ExpressionAst = parse_expression();
  LetAst.get().add_child(ExpressionAst);

  require_token(current_token, TokenValue_t::J_SEMICOLON);
  get_next_token();

  return LetAst;
}

AstNodeRef Parser::parse_do_statement()
{
  require_token(current_token, TokenValue_t::J_DO);
  AstNodeRef do_ast = create_ast_node(AstNodeType_t::N_DO_STATEMENT);
  get_next_token();

  AstNodeRef const call_ast = parse_subroutine_call();

  require_token(current_token, TokenValue_t::J_SEMICOLON);
  get_next_token();

  do_ast.get().add_child(call_ast);

  return do_ast;
}

AstNodeRef Parser::parse_return_statement()
{
  const auto start_token = TokenValue_t::J_RETURN;
  assert(current_token.get().value_enum == start_token);

  AstNodeRef ReturnAst = create_ast_node(AstNodeType_t::N_RETURN_STATEMENT);
  get_next_token();

  if (current_token.get().value_enum != TokenValue_t::J_SEMICOLON)
  {
    AstNode& ExpressionAst = parse_expression();
    ReturnAst.get().add_child(ExpressionAst);
  }

  require_token(current_token, TokenValue_t::J_SEMICOLON);
  get_next_token();

  return ReturnAst;
}

// <while-statement> ::= "while" "(" <expression> ")" "{" {<statement>}* "}"
AstNodeRef Parser::parse_while_statement()
{
  require_token(current_token, TokenValue_t::J_WHILE);
  get_next_token();

  require_token(current_token, TokenValue_t::J_LEFT_PARENTHESIS);
  get_next_token();

  AstNodeRef while_ast = create_ast_node(AstNodeType_t::N_WHILE_STATEMENT);
  while_ast.get().add_child(parse_expression());

  require_token(current_token, TokenValue_t::J_RIGHT_PARENTHESIS);
  get_next_token();

  require_token(current_token, TokenValue_t::J_LEFT_BRACE);
  get_next_token();

  AstNodeRef const statement_block_root = parse_inner_statements();
  while_ast.get().add_child(statement_block_root);

  require_token(current_token, TokenValue_t::J_RIGHT_BRACE);
  get_next_token();

  return while_ast;
}

// <if-statement> ::= "if" "(" <expression ")" "{" {<statement>}* "}"
//                    {"else" "{" {<statement>}* "}"}?
AstNodeRef Parser::parse_if_statement()
{
  require_token(current_token, TokenValue_t::J_IF);
  get_next_token();

  require_token(current_token, TokenValue_t::J_LEFT_PARENTHESIS);
  get_next_token();

  AstNodeRef if_ast = create_ast_node(AstNodeType_t::N_IF_STATEMENT);
  if_ast.get().add_child(parse_expression());

  require_token(current_token, TokenValue_t::J_RIGHT_PARENTHESIS);
  get_next_token();

  require_token(current_token, TokenValue_t::J_LEFT_BRACE);
  get_next_token();

  AstNodeRef const true_statement_block_root = parse_inner_statements();
  if_ast.get().add_child(true_statement_block_root);

  require_token(current_token, TokenValue_t::J_RIGHT_BRACE);
  get_next_token();

  if (current_token.get().value_enum == TokenValue_t::J_ELSE)
  {
    require_token(current_token, TokenValue_t::J_ELSE);
    get_next_token();

    require_token(current_token, TokenValue_t::J_LEFT_BRACE);
    get_next_token();

    AstNodeRef const false_statement_block_root = parse_inner_statements();
    if_ast.get().add_child(false_statement_block_root);

    require_token(current_token, TokenValue_t::J_RIGHT_BRACE);
    get_next_token();
  }

  return if_ast;
}

// <subroutine-call>  ::= <call-site> "(" {<call-arguments>}? ")"
// <call-site>        ::= <subroutine-name> |
//                        (<class-name> | <var-name>) "." <subroutine-name>
// <subroutine-name>  ::= <identifier>
AstNodeRef Parser::parse_subroutine_call()
{
  require_token(current_token, TokenValue_t::J_IDENTIFIER);

  if ((peek_token.get().value_enum != TokenValue_t::J_PERIOD) &&
      (peek_token.get().value_enum != TokenValue_t::J_LEFT_PARENTHESIS))
  {
    return AST.get_empty_node_ref();
  }

  AstNodeRef root_node = create_ast_node(AstNodeType_t::N_SUBROUTINE_CALL);

  if (peek_token.get().value_enum == TokenValue_t::J_PERIOD)
  {
    AstNodeRef const global_call_node =
        create_ast_node(AstNodeType_t::N_GLOBAL_CALL_SITE);

    global_call_node.get().add_child(create_ast_node(
        AstNodeType_t::N_GLOBAL_BIND_NAME, current_token.get().value_str));
    get_next_token();

    require_token(current_token, TokenValue_t::J_PERIOD);
    get_next_token();

    global_call_node.get().add_child(create_ast_node(
        AstNodeType_t::N_SUBROUTINE_NAME, current_token.get().value_str));
    get_next_token();

    root_node.get().add_child(global_call_node);
  }
  else
  {
    AstNodeRef const local_call_node =
        create_ast_node(AstNodeType_t::N_LOCAL_CALL_SITE);
    local_call_node.get().add_child(create_ast_node(
        AstNodeType_t::N_SUBROUTINE_NAME, current_token.get().value_str));
    get_next_token();

    root_node.get().add_child(local_call_node);
  }

  require_token(current_token, TokenValue_t::J_LEFT_PARENTHESIS);
  get_next_token();

  AstNodeRef const call_arguments_node =
      create_ast_node(AstNodeType_t::N_CALL_ARGUMENTS);
  root_node.get().add_child(call_arguments_node);

  for (;;)
  {
    if (current_token.get().value_enum == TokenValue_t::J_RIGHT_PARENTHESIS)
    {
      break;
    }

    AstNodeRef const statement_ast = parse_expression();

    if (statement_ast.get() != AST.get_empty_node_ref().get())
    {
      call_arguments_node.get().add_child(statement_ast);
    }

    if (current_token.get().value_enum == TokenValue_t::J_COMMA)
    {
      get_next_token();
    }
  }

  require_token(current_token, TokenValue_t::J_RIGHT_PARENTHESIS);
  get_next_token();

  return root_node;
}

// <scalar-var>       ::= <var-name>
// <array-var>        ::= <var-name> "[" <expression> "]"
AstNodeRef Parser::parse_variable()
{
  require_token(current_token, TokenValue_t::J_IDENTIFIER);

  AstNodeRef variable_node = AST.get_empty_node_ref();

  if (peek_token.get().value_enum == TokenValue_t::J_LEFT_BRACKET)
  {
    variable_node = create_ast_node(AstNodeType_t::N_SUBSCRIPTED_VARIABLE_NAME,
                                    current_token.get().value_str);
    get_next_token();

    require_token(current_token, TokenValue_t::J_LEFT_BRACKET);
    get_next_token();

    AstNodeRef const subscript_node = parse_expression();

    require_token(current_token, TokenValue_t::J_RIGHT_BRACKET);
    get_next_token();

    variable_node.get().add_child(subscript_node);
  }
  else
  {
    variable_node = create_ast_node(AstNodeType_t::N_VARIABLE_NAME,
                                    current_token.get().value_str);
    get_next_token();
  }

  return variable_node;
}

AstNodeRef Parser::parse_type(AstNodeType_t node_type)
{
  AstNodeRef rtn_type_node = AST.get_empty_node_ref();

  if ((node_type == AstNodeType_t::N_RETURN_TYPE) &&
      (current_token.get().value_enum == TokenValue_t::J_VOID))
  {
    // Used by <subroutine-decl>
    rtn_type_node = create_ast_node(node_type, "void");
  }
  else if (current_token.get().value_enum == TokenValue_t::J_INT)
  {
    rtn_type_node = create_ast_node(node_type, "int");
  }
  else if (current_token.get().value_enum == TokenValue_t::J_CHAR)
  {
    rtn_type_node = create_ast_node(node_type, "char");
  }
  else if (current_token.get().value_enum == TokenValue_t::J_BOOLEAN)
  {
    rtn_type_node = create_ast_node(node_type, "boolean");
  }
  else if (current_token.get().value_enum == TokenValue_t::J_IDENTIFIER)
  {
    rtn_type_node = create_ast_node(node_type, current_token.get().value_str);
  }

  if (rtn_type_node.get() != AST.get_empty_node_ref().get())
  {
    get_next_token();
  }

  return rtn_type_node;
}

AstNodeRef Parser::parse_term()
{
  AstNodeRef TermAst = AST.get_empty_node_ref();

  if (current_token.get().value_enum == TokenValue_t::J_INTEGER_CONSTANT)
  {
    int const int_const = std::stoi(current_token.get().value_str);

    TermAst = create_ast_node(AstNodeType_t::N_INTEGER_CONSTANT, int_const);
    get_next_token();
  }
  else if (current_token.get().value_enum == TokenValue_t::J_STRING_CONSTANT)
  {
    TermAst = create_ast_node(AstNodeType_t::N_STRING_CONSTANT,
                              current_token.get().value_str);
    get_next_token();
  }
  else if (current_token.get().value_enum == TokenValue_t::J_TRUE)
  {
    TermAst = create_ast_node(AstNodeType_t::N_TRUE_KEYWORD);
    get_next_token();
  }
  else if (current_token.get().value_enum == TokenValue_t::J_FALSE)
  {
    TermAst = create_ast_node(AstNodeType_t::N_FALSE_KEYWORD);
    get_next_token();
  }
  else if (current_token.get().value_enum == TokenValue_t::J_NULL)
  {
    TermAst = create_ast_node(AstNodeType_t::N_NULL_KEYWORD);
    get_next_token();
  }
  else if (current_token.get().value_enum == TokenValue_t::J_THIS)
  {
    TermAst = create_ast_node(AstNodeType_t::N_THIS_KEYWORD);
    get_next_token();
  }
  else if (current_token.get().value_enum == TokenValue_t::J_LEFT_PARENTHESIS)
  {
    get_next_token();
    TermAst = parse_expression();
    require_token(current_token, TokenValue_t::J_RIGHT_PARENTHESIS);
    get_next_token();
  }
  else if (current_token.get().value_enum == TokenValue_t::J_MINUS)
  {
    AstNodeRef const prefix_neg =
        create_ast_node(AstNodeType_t::N_OP_PREFIX_NEG);
    get_next_token();
    AstNodeRef const prefix_term = parse_term();
    prefix_neg.get().add_child(prefix_term);
    TermAst = prefix_neg;
  }
  else if (current_token.get().value_enum == TokenValue_t::J_TILDE)
  {
    AstNodeRef const prefix_not =
        create_ast_node(AstNodeType_t::N_OP_PREFIX_LOGICAL_NOT);
    get_next_token();
    AstNodeRef const prefix_term = parse_term();
    prefix_not.get().add_child(prefix_term);
    TermAst = prefix_not;
  }
  else if (current_token.get().value_enum == TokenValue_t::J_IDENTIFIER)
  {
    TermAst = parse_subroutine_call();

    if (TermAst.get().type == AST.get_empty_node_ref().get().type)
    {
      // array term
      if (peek_token.get().value_enum == TokenValue_t::J_LEFT_BRACKET)
      {
        TermAst = create_ast_node(AstNodeType_t::N_SUBSCRIPTED_VARIABLE_NAME,
                                  current_token.get().value_str);
        get_next_token();

        require_token(current_token.get().value_enum,
                      TokenValue_t::J_LEFT_BRACKET);
        get_next_token();

        TermAst.get().add_child(parse_expression());
        require_token(current_token.get().value_enum,
                      TokenValue_t::J_RIGHT_BRACKET);
        get_next_token();
      }
      // scalar term
      else
      {
        TermAst = create_ast_node(AstNodeType_t::N_VARIABLE_NAME,
                                  current_token.get().value_str);
        get_next_token();
      }
    }
  }

  if (TermAst.get().type == AST.get_empty_node_ref().get().type)
  {
    std::stringstream ss;

    ss << "Line #" << current_token.get().line_number << ": "
       << "Expected TERM token while parsing "
       << JackToken::to_string(current_token.get().value_enum);

    fatal_error(ss.str());
  }

  return TermAst;
}

//
// Expression parsers
//

AstNodeRef Parser::parse_expression()
{
  // sub-expression precedence level that will be parsed
  using PrecedenceLevel_t = enum class PrecedenceLevel_s {
    P_OR,
    P_AND,
    P_CMP,  // includes "<", ">", and "="
    P_ADD,  // includes "+" and "-"
    P_MUL,  // includes "*" and "/"
    P_TERM
  };

  // retuns the next higher precedence level in the precedence chain
  auto next_in_precedence_level =
      [](const PrecedenceLevel_t& precedence_level) -> PrecedenceLevel_t {
    assert((precedence_level != PrecedenceLevel_t::P_TERM) &&
           "No next precedence level after P_TERM");

    switch (precedence_level)
    {
      case PrecedenceLevel_t::P_OR:
        return (PrecedenceLevel_t::P_AND);
      case PrecedenceLevel_t::P_AND:
        return (PrecedenceLevel_t::P_CMP);
      case PrecedenceLevel_t::P_CMP:
        return (PrecedenceLevel_t::P_ADD);
      case PrecedenceLevel_t::P_ADD:
        return (PrecedenceLevel_t::P_MUL);
      case PrecedenceLevel_t::P_MUL:
        return (PrecedenceLevel_t::P_TERM);
      case PrecedenceLevel_t::P_TERM:
        return (PrecedenceLevel_t::P_TERM);
    }

    return (PrecedenceLevel_t::P_TERM);
  };

  // tokens that map to each operation's precedence level
  using OperationOpMap_t =
      std::map<PrecedenceLevel_t, const std::unordered_set<TokenValue_t>>;

  OperationOpMap_t OperationOpMap {
      // clang-format off
      {PrecedenceLevel_t::P_OR,   {TokenValue_t::J_VBAR}},
      {PrecedenceLevel_t::P_AND,  {TokenValue_t::J_AMPERSAND}},
      {PrecedenceLevel_t::P_CMP,  {TokenValue_t::J_LESS_THAN,
                                   TokenValue_t::J_GREATER_THAN,
                                   TokenValue_t::J_EQUAL}},
      {PrecedenceLevel_t::P_ADD,  {TokenValue_t::J_PLUS, TokenValue_t::J_MINUS}},
      {PrecedenceLevel_t::P_MUL,  {TokenValue_t::J_ASTERISK, TokenValue_t::J_DIVIDE}},
      {PrecedenceLevel_t::P_TERM, {}},
      // clang-format on
  };

  if (left_associative_expressions)
  {
    OperationOpMap = OperationOpMap_t {
        // clang-format off
        {PrecedenceLevel_t::P_OR,  {}},
        {PrecedenceLevel_t::P_AND, {}},
        {PrecedenceLevel_t::P_CMP, {}},
        {PrecedenceLevel_t::P_ADD, {}},
        {PrecedenceLevel_t::P_MUL, {TokenValue_t::J_VBAR, TokenValue_t::J_AMPERSAND,
                                    TokenValue_t::J_LESS_THAN, TokenValue_t::J_GREATER_THAN,
                                    TokenValue_t::J_EQUAL, TokenValue_t::J_PLUS,
                                    TokenValue_t::J_MINUS,
                                    TokenValue_t::J_ASTERISK, TokenValue_t::J_DIVIDE}},
        {PrecedenceLevel_t::P_TERM, {}},
        // clang-format on
    };
  }

  using TokenToAstMap_t = std::map<TokenValue_t, AstNodeType_t>;

  TokenToAstMap_t TokenAstMap {
      {TokenValue_t::J_DIVIDE, AstNodeType_t::N_OP_DIVIDE},
      {TokenValue_t::J_ASTERISK, AstNodeType_t::N_OP_MULTIPLY},
      {TokenValue_t::J_MINUS, AstNodeType_t::N_OP_SUBTRACT},
      {TokenValue_t::J_PLUS, AstNodeType_t::N_OP_ADD},
      {TokenValue_t::J_EQUAL, AstNodeType_t::N_OP_LOGICAL_EQUALS},
      {TokenValue_t::J_GREATER_THAN, AstNodeType_t::N_OP_LOGICAL_GT},
      {TokenValue_t::J_LESS_THAN, AstNodeType_t::N_OP_LOGICAL_LT},
      {TokenValue_t::J_AMPERSAND, AstNodeType_t::N_OP_LOGICAL_AND},
      {TokenValue_t::J_VBAR, AstNodeType_t::N_OP_LOGICAL_OR}};

  // internal lambda decls
  std::function<AstNodeRef(const PrecedenceLevel_t&)> parse_subexpression;
  std::function<AstNodeRef(AstNodeRef, const PrecedenceLevel_t&)>
      combine_oper_chain;

  // Construction of the recursive expression parsers supporting operator
  // precedence grammar.  The <or-expr> definition is shown as an example.
  // Each subexpressions, such as <or-expr>, decomposes to the higher
  // subexpression in the precedence chain.  In the case of <or-expr> this
  // is <and-expr>.
  //
  //               parse_subexpression(and-level)
  //                            /
  //                           /      parse_subexpression(and-level)
  //                          /                 |
  //                    /--------\              |
  //     <or-expr>  ::= <and-expr> {<or-op> <and-expr>}+
  //                               \------------------/
  //                                        /
  //                                       /
  //                            combine_oper_chain(or-level)

  parse_subexpression =
      [&](const PrecedenceLevel_t& subexp_precedence_level) -> AstNodeRef {
    // combine_oper_chain - builds the RHS chain of operations of the same
    // precedence level; returns EmptyNodeRef if the current_token isn't
    // a valid operator at the required precedence level.
    combine_oper_chain =
        [&](AstNodeRef lhs, const PrecedenceLevel_t& prec_level) -> AstNodeRef {
      // operators_in_level - set of operators at current precedence level
      const auto& operators_in_level = OperationOpMap[prec_level];

      // recursive base case - is current_token valid for precedence level?
      if (!operators_in_level.contains(current_token.get().value_enum))
      {
        return AST.get_empty_node_ref();
      }

      // consume the operator token at this point so that on the recursive
      // return, it will become the root node of the parsed subexpression.
      const auto& operator_token = current_token.get().value_enum;
      get_next_token();

      AstNodeRef chained_root = create_ast_node(TokenAstMap[operator_token]);

      chained_root.get().add_child(lhs);

      if (AstNodeRef const rhs = parse_subexpression(prec_level);
          rhs.get() != AST.get_empty_node_ref().get())
      {
        chained_root.get().add_child(rhs);
      }

      return chained_root;
    };  // parse_oper_chain() lambda

    //
    // parse_subexpression() starts here
    //

    AstNodeRef lhs = AST.get_empty_node_ref();

    // recursive base case
    if (subexp_precedence_level == PrecedenceLevel_t::P_MUL)
    {
      lhs = parse_term();
    }
    else
    {
      const PrecedenceLevel_t next_subexpression_oper =
          next_in_precedence_level(subexp_precedence_level);

      // recurse to higher precedence expression
      lhs = parse_subexpression(next_subexpression_oper);
    }

    for (bool done = false; !done; /* empty */)
    {
      AstNodeRef const new_lhs =
          combine_oper_chain(lhs, subexp_precedence_level);

      if (new_lhs.get() == AST.get_empty_node_ref().get())
      {
        done = true;
      }
      else
      {
        lhs = new_lhs;
      }
    }

    return lhs;
  };  // parse_subexpression()

  //
  // parse_expression() starts here
  //

  AstNodeRef ExpressionAst = AST.get_empty_node_ref();

  if (!left_associative_expressions)
  {
    // lowest operation precedence level is P_OR
    ExpressionAst = parse_subexpression(PrecedenceLevel_t::P_OR);
  }
  else
  {
    // when not adhering to operator precedence, use P_MUL when internally
    // is the recursive base case
    ExpressionAst = parse_subexpression(PrecedenceLevel_t::P_MUL);
  }

  return ExpressionAst;
}

}  // namespace jfcl
