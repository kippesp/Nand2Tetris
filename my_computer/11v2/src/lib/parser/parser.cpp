#include "parser.h"

#include <cassert>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <unordered_set>

#include <signal.h>

using namespace ast;

namespace recursive_descent {

Parser::Parser(Tokens_t& tokens)
    : token_iter(tokens.begin()),
      token_iter_end(tokens.end()),
      current_token(*token_iter),
      peek_token(*(++token_iter)),
      EmptyNode(
          std::make_unique<ast::AstNode>(ast::AstNodeType_t::N_UNDEFINED)),
      EmptyNodeRef(*EmptyNode)
{
}

void Parser::require_token(TokenValue_t start_token, TokenValue_t token_value)
{
  if (current_token.get().value_enum != token_value)
  {
    std::stringstream ss;

    ss << "Line #" << current_token.get().line_number << ": "
       << "Expected " << JackToken::to_string(token_value) << " while parsing "
       << JackToken::to_string(start_token);

#ifndef WIN32
    raise(SIGTRAP);
#endif
    fatal_error(ss.str());
  }
}

AstNodeRef Parser::create_ast_node(AstNodeType_t type)
{
  return AST.add(AstNode(type));
}

AstNodeRef Parser::parse_class()
{
  const auto start_token = TokenValue_t::J_CLASS;
  assert(current_token.get().value_enum == start_token);

  AstNodeRef ClassAst = create_ast_node(AstNodeType_t::N_CLASS_DECL);

  get_next_token();
  require_token(start_token, TokenValue_t::J_IDENTIFIER);

  ClassAst.get().value = current_token.get().value_str;

  get_next_token();
  require_token(start_token, TokenValue_t::J_LEFT_BRACE);
  get_next_token();

  // <class-var-decl>
  //      ::= ("static" | "field") <type> <var-name> {"," <var-name>}* ";"

  // raise(SIGTRAP);
  if ((current_token.get().value_enum == TokenValue_t::J_STATIC) ||
      (current_token.get().value_enum == TokenValue_t::J_FIELD))
  {
    AstNodeRef class_vars_block_node = parse_classvar_decl_block();

    ClassAst.get().add_child(class_vars_block_node);
  }

  while (current_token.get().value_enum != TokenValue_t::J_RIGHT_BRACE)
  {
    AstNodeRef SubroutineAstNodeRef = parse_subroutine();
    ClassAst.get().add_child(SubroutineAstNodeRef);
  }

  require_token(start_token, TokenValue_t::J_RIGHT_BRACE);

  return ClassAst;
}

AstNodeRef Parser::parse_classvar_decl_block()
{
  const auto start_token = TokenValue_t::J_CLASS;

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
         << "Expected static|field"
         << " while parsing " << JackToken::to_string(start_token);

      fatal_error(ss.str());
    }

    get_next_token();

    std::string var_type;

    if (current_token.get().value_enum == TokenValue_t::J_INT)
    {
      var_type = "int";
    }
    else if (current_token.get().value_enum == TokenValue_t::J_CHAR)
    {
      var_type = "char";
    }
    else if (current_token.get().value_enum == TokenValue_t::J_BOOLEAN)
    {
      var_type = "boolean";
    }
    else
    {
      require_token(start_token, TokenValue_t::J_IDENTIFIER);
      var_type = current_token.get().value_str;
    }

    auto add_def_to_block = [&]() {
      require_token(start_token, TokenValue_t::J_IDENTIFIER);

      AstNodeRef class_var_decl = create_ast_node(
          AstNodeType_t::N_VARIABLE_DECL, current_token.get().value_str);

      class_var_decl.get().add_child(
          create_ast_node(AstNodeType_t::N_CLASS_VARIABLE_SCOPE, var_scope));

      class_var_decl.get().add_child(
          create_ast_node(AstNodeType_t::N_VARIABLE_TYPE, var_type));

      class_vars_block_node.get().add_child(class_var_decl);
      get_next_token();
    };

    get_next_token();
    require_token(start_token, TokenValue_t::J_IDENTIFIER);

    add_def_to_block();

    while (current_token.get().value_enum == TokenValue_t::J_COMMA)
    {
      get_next_token();
      require_token(start_token, TokenValue_t::J_IDENTIFIER);
      add_def_to_block();
    }

    require_token(start_token, TokenValue_t::J_SEMICOLON);
    get_next_token();
  }

  return class_vars_block_node;
}

AstNodeRef Parser::parse_subroutine()
{
  auto start_token = TokenValue_t::J_UNDEFINED;
  std::optional<AstNodeRef> SubrDeclAst;

  if (current_token.get().value_enum == TokenValue_t::J_FUNCTION)
  {
    start_token = TokenValue_t::J_FUNCTION;
    SubrDeclAst = create_ast_node(AstNodeType_t::N_FUNCTION_DECL);
  }
  else if (current_token.get().value_enum == TokenValue_t::J_METHOD)
  {
    start_token = TokenValue_t::J_METHOD;
    SubrDeclAst = create_ast_node(AstNodeType_t::N_METHOD_DECL);
  }
  else if (current_token.get().value_enum == TokenValue_t::J_CONSTRUCTOR)
  {
    start_token = TokenValue_t::J_CONSTRUCTOR;
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

  AstNodeRef SubrDescrAst = create_ast_node(AstNodeType_t::N_SUBROUTINE_DESCR);

  // <subroutine-decl>  ::= ("constructor" | "function" | "method")
  //                        ("void" | <type>) <subroutine-name>
  //                        \---------------/
  //                                /
  //                               /
  //                     SubrDeclReturnTypeAst

  std::optional<AstNodeRef> SubrDeclReturnTypeAst;

  if (current_token.get().value_enum == TokenValue_t::J_IDENTIFIER)
  {
    SubrDeclReturnTypeAst = create_ast_node(AstNodeType_t::N_RETURN_TYPE,
                                            current_token.get().value_str);
  }
  else if (current_token.get().value_enum == TokenValue_t::J_INT)
  {
    SubrDeclReturnTypeAst =
        create_ast_node(AstNodeType_t::N_RETURN_TYPE, "int");
  }
  else if (current_token.get().value_enum == TokenValue_t::J_CHAR)
  {
    SubrDeclReturnTypeAst =
        create_ast_node(AstNodeType_t::N_RETURN_TYPE, "char");
  }
  else if (current_token.get().value_enum == TokenValue_t::J_BOOLEAN)
  {
    SubrDeclReturnTypeAst =
        create_ast_node(AstNodeType_t::N_RETURN_TYPE, "boolean");
  }
  else if (current_token.get().value_enum == TokenValue_t::J_VOID)
  {
    SubrDeclReturnTypeAst =
        create_ast_node(AstNodeType_t::N_RETURN_TYPE, "void");
  }
  else
  {
    std::stringstream ss;

    ss << "Line #" << current_token.get().line_number << ": "
       << "Expected int|char|boolean|void|ClassName"
       << " while parsing " << JackToken::to_string(start_token);

    fatal_error(ss.str());
  }

  SubrDescrAst.get().add_child(SubrDeclReturnTypeAst.value());

  SubrDeclAst.value().get().add_child(SubrDescrAst);

  // <subroutine-decl>  ::= ("constructor" | "function" | "method")
  //                        ("void" | <type>) <subroutine-name>
  //                                          \---------------/
  //                                                 /
  //                                                /
  //                                      SubrDeclAst->value

  get_next_token();
  require_token(start_token, TokenValue_t::J_IDENTIFIER);

  SubrDeclAst.value().get().value = current_token.get().value_str;

  get_next_token();
  require_token(start_token, TokenValue_t::J_LEFT_PARENTHESIS);
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
  // /----------------\        /-----------------\
  // <input-parameters>   ::= {(<type> <var-name>) {"," <type> <var-name>}*}?
  //       \
  //        -- N_INPUT_PARAMETERS

  if (current_token.get().value_enum != TokenValue_t::J_RIGHT_PARENTHESIS)
  {
    AstNodeRef input_parms_root =
        create_ast_node(AstNodeType_t::N_INPUT_PARAMETERS);

    while (current_token.get().value_enum != TokenValue_t::J_RIGHT_PARENTHESIS)
    {
      std::optional<AstNodeRef> parm_type_node;

      // TODO: pattern is very common
      if (current_token.get().value_enum == TokenValue_t::J_INT)
      {
        parm_type_node = create_ast_node(AstNodeType_t::N_VARIABLE_TYPE, "int");
      }
      else if (current_token.get().value_enum == TokenValue_t::J_CHAR)
      {
        parm_type_node =
            create_ast_node(AstNodeType_t::N_VARIABLE_TYPE, "char");
      }
      else if (current_token.get().value_enum == TokenValue_t::J_BOOLEAN)
      {
        parm_type_node =
            create_ast_node(AstNodeType_t::N_VARIABLE_TYPE, "boolean");
      }
      else
      {
        require_token(start_token, TokenValue_t::J_IDENTIFIER);
        parm_type_node = create_ast_node(AstNodeType_t::N_VARIABLE_TYPE,
                                         current_token.get().value_str);
      }

      get_next_token();
      require_token(start_token, TokenValue_t::J_IDENTIFIER);

      AstNodeRef variable_decl_root = create_ast_node(
          AstNodeType_t::N_VARIABLE_DECL, current_token.get().value_str);

      variable_decl_root.get().add_child(parm_type_node.value());

      input_parms_root.get().add_child(variable_decl_root);

      get_next_token();

      if (current_token.get().value_enum == TokenValue_t::J_COMMA)
      {
        get_next_token();
      }
    }

    SubrDescrAst.get().add_child(input_parms_root);
  }

  require_token(start_token, TokenValue_t::J_RIGHT_PARENTHESIS);
  get_next_token();

  require_token(start_token, TokenValue_t::J_LEFT_BRACE);
  get_next_token();

  //
  // <subroutine-body>  ::= "{" {<var-decl>}* {<statement>}* "}"
  //                            \----------/
  //                                   \
  //                                    ------- N_LOCAL_VARIABLES
  //
  //                                        ------ N_VARIABLE_DECL
  //                                       /
  //                              /---------------\
  // <var-decl>         ::= "var" <type> <var-name> {"," <var-name>}* ";"
  //                              \----/
  //                                /
  //            N_VARIABLE_TYPE ----

  AstNodeRef SubroutineBodyAst =
      create_ast_node(AstNodeType_t::N_SUBROUTINE_BODY);

  if (current_token.get().value_enum == TokenValue_t::J_VAR)
  {
    AstNodeRef SubroutineVarDeclBlockAst =
        create_ast_node(AstNodeType_t::N_LOCAL_VARIABLES);

    while (current_token.get().value_enum == TokenValue_t::J_VAR)
    {
      std::optional<AstNodeRef> VarTypeAst;

      get_next_token();

      if (current_token.get().value_enum == TokenValue_t::J_INT)
      {
        VarTypeAst = create_ast_node(AstNodeType_t::N_VARIABLE_TYPE, "int");
      }
      else if (current_token.get().value_enum == TokenValue_t::J_CHAR)
      {
        VarTypeAst = create_ast_node(AstNodeType_t::N_VARIABLE_TYPE, "char");
      }
      else if (current_token.get().value_enum == TokenValue_t::J_BOOLEAN)
      {
        VarTypeAst = create_ast_node(AstNodeType_t::N_VARIABLE_TYPE, "boolean");
      }
      else
      {
        require_token(start_token, TokenValue_t::J_IDENTIFIER);
        VarTypeAst = create_ast_node(AstNodeType_t::N_VARIABLE_TYPE,
                                     current_token.get().value_str);
      }

      do
      {
        get_next_token();
        require_token(start_token, TokenValue_t::J_IDENTIFIER);

        AstNodeRef SubroutineVarDeclAst = create_ast_node(
            AstNodeType_t::N_VARIABLE_DECL, current_token.get().value_str);

        SubroutineVarDeclAst.get().add_child(VarTypeAst.value());

        SubroutineVarDeclBlockAst.get().add_child(SubroutineVarDeclAst);
        get_next_token();
      } while (current_token.get().value_enum == TokenValue_t::J_COMMA);

      require_token(start_token, TokenValue_t::J_SEMICOLON);
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
    AstNodeRef StatementBlockAst =
        create_ast_node(AstNodeType_t::N_STATEMENT_BLOCK);

    // TODO: permit no statements to have no block

    while (current_token.get().value_enum != TokenValue_t::J_RIGHT_BRACE)
    {
      AstNodeRef StatementAst = parse_statement();
      assert((StatementAst.get() != EmptyNodeRef.get()) &&
             "statement required");
      StatementBlockAst.get().add_child(StatementAst);
    }

    SubroutineBodyAst.get().add_child(StatementBlockAst);
  }

  SubrDeclAst.value().get().add_child(SubroutineBodyAst);

  require_token(start_token, TokenValue_t::J_RIGHT_BRACE);
  get_next_token();

  return SubrDeclAst.value();
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

  assert(StatementAst.has_value());

  return StatementAst.value();
}

AstNodeRef Parser::parse_let_statement()
{
  const auto start_token = TokenValue_t::J_LET;
  assert(current_token.get().value_enum == start_token);

  AstNodeRef LetAst = create_ast_node(AstNodeType_t::N_LET_STATEMENT);
  get_next_token();

  require_token(start_token, TokenValue_t::J_IDENTIFIER);

  LetAst.get().add_child(parse_variable());

  require_token(start_token, TokenValue_t::J_EQUAL);
  get_next_token();

  AstNode& ExpressionAst = parse_expression();
  LetAst.get().add_child(ExpressionAst);

  require_token(start_token, TokenValue_t::J_SEMICOLON);
  get_next_token();

  return LetAst;
}

AstNodeRef Parser::parse_do_statement()
{
  require_token(current_token.get().value_enum, TokenValue_t::J_DO);
  get_next_token();

  AstNodeRef call_ast = parse_subroutine_call();

  require_token(current_token.get().value_enum, TokenValue_t::J_SEMICOLON);
  get_next_token();

  AstNodeRef do_ast = create_ast_node(AstNodeType_t::N_DO_STATEMENT);
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

  require_token(start_token, TokenValue_t::J_SEMICOLON);
  get_next_token();

  return ReturnAst;
}

// <subroutine-call>  ::= <call-site> "(" {<expression-list>}? ")"
// <call-site>        ::= <subroutine-name> |
//                        (<class-name> | <var-name>) "." <subroutine-name>
// <subroutine-name>  ::= <identifier>
AstNodeRef Parser::parse_subroutine_call()
{
  require_token(current_token.get().value_enum, TokenValue_t::J_IDENTIFIER);

  if ((peek_token.get().value_enum != TokenValue_t::J_PERIOD) &&
      (peek_token.get().value_enum != TokenValue_t::J_LEFT_PARENTHESIS))
  {
    return EmptyNodeRef;
  }

  AstNodeRef root_node = create_ast_node(AstNodeType_t::N_SUBROUTINE_CALL);

  if (peek_token.get().value_enum == TokenValue_t::J_PERIOD)
  {
    AstNodeRef global_call_node =
        create_ast_node(AstNodeType_t::N_GLOBAL_CALL_SITE);

    global_call_node.get().add_child(create_ast_node(
        AstNodeType_t::N_GLOBAL_BIND_NAME, current_token.get().value_str));
    get_next_token();

    require_token(current_token.get().value_enum, TokenValue_t::J_PERIOD);
    get_next_token();

    global_call_node.get().add_child(create_ast_node(
        AstNodeType_t::N_SUBROUTINE_NAME, current_token.get().value_str));
    get_next_token();

    root_node.get().add_child(global_call_node);
  }
  else
  {
    AstNodeRef local_call_node =
        create_ast_node(AstNodeType_t::N_LOCAL_CALL_SITE);
    local_call_node.get().add_child(create_ast_node(
        AstNodeType_t::N_SUBROUTINE_NAME, current_token.get().value_str));
    get_next_token();

    root_node.get().add_child(local_call_node);
  }

  require_token(current_token.get().value_enum,
                TokenValue_t::J_LEFT_PARENTHESIS);
  get_next_token();

  for (bool done = false; !done; /* none */)
  {
    if (current_token.get().value_enum == TokenValue_t::J_RIGHT_PARENTHESIS)
    {
      done = true;
      break;
    }

    AstNodeRef statement_ast = parse_expression();

    if (statement_ast.get() != EmptyNodeRef.get())
    {
      root_node.get().add_child(statement_ast);
    }

    if (current_token.get().value_enum == TokenValue_t::J_COMMA)
    {
      get_next_token();
    }
  }

  require_token(current_token.get().value_enum,
                TokenValue_t::J_RIGHT_PARENTHESIS);
  get_next_token();

  return root_node;
}

// <scalar-var>       ::= <var-name>
// <array-var>        ::= <var-name> "[" <expression> "]"
AstNodeRef Parser::parse_variable()
{
  require_token(current_token.get().value_enum, TokenValue_t::J_IDENTIFIER);

  AstNodeRef variable_node = EmptyNodeRef;

  if (peek_token.get().value_enum == TokenValue_t::J_LEFT_BRACKET)
  {
    variable_node = create_ast_node(AstNodeType_t::N_SUBSCRIPTED_VARIABLE_NAME,
                                    current_token.get().value_str);
    get_next_token();

    require_token(current_token.get().value_enum, TokenValue_t::J_LEFT_BRACKET);
    get_next_token();

    AstNodeRef subscript_node = parse_expression();

    require_token(current_token.get().value_enum,
                  TokenValue_t::J_RIGHT_BRACKET);
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

//
// Expression parsers
//

AstNodeRef Parser::parse_expression()
{
  // sub-expression op_group that will be parsed
  using OperationGroup_t = enum class OperationGroup_s {
    P_OR,
    P_AND,
    P_CMP,
    P_ADD,
    P_MUL,
    P_TERM
  };

  // retuns the next higher operation group in the precedence chain
  auto next_in_oper_group =
      [](const OperationGroup_t& oper_group) -> OperationGroup_t {
    assert((oper_group != OperationGroup_t::P_TERM) &&
           "No next operation group after P_TERM");

    switch (oper_group)
    {
      case OperationGroup_t::P_OR:
        return (OperationGroup_t::P_AND);
      case OperationGroup_t::P_AND:
        return (OperationGroup_t::P_CMP);
      case OperationGroup_t::P_CMP:
        return (OperationGroup_t::P_ADD);
      case OperationGroup_t::P_ADD:
        return (OperationGroup_t::P_MUL);
      case OperationGroup_t::P_MUL:
        return (OperationGroup_t::P_TERM);
      case OperationGroup_t::P_TERM:
        return (OperationGroup_t::P_TERM);
    }

    return (OperationGroup_t::P_TERM);
  };

  // tokens that map to each op_group
  using OperationOpMap_t =
      std::map<OperationGroup_t, const std::unordered_set<TokenValue_t>>;

  OperationOpMap_t OperationOpMap {
      // clang-format off
      {OperationGroup_t::P_OR,   {TokenValue_t::J_VBAR}},
      {OperationGroup_t::P_AND,  {TokenValue_t::J_AMPERSAND}},
      {OperationGroup_t::P_CMP,  {TokenValue_t::J_LESS_THAN,
                                  TokenValue_t::J_GREATER_THAN,
                                  TokenValue_t::J_EQUAL}},
      {OperationGroup_t::P_ADD,  {TokenValue_t::J_PLUS, TokenValue_t::J_MINUS}},
      {OperationGroup_t::P_MUL,  {TokenValue_t::J_ASTERISK, TokenValue_t::J_DIVIDE}},
      {OperationGroup_t::P_TERM, {}},
      // clang-format on
  };

  // Construction of the recursive expression parsers supporting operator
  // precedence grammar.  The <or-expr> definition is shown as an example.
  // Each subexpressions, such as <or-expr>, decomposes to the next higher
  // subexpression in the precedence chain.  In the case of <or-expr> this
  // is <and-expr>.
  //
  //                          recurse with
  //                     parse_subexpression()
  //                               /
  //                              /      recurse with
  //                             /    parse_oper_chain(LHS)
  //                            /            /
  //                    /-------------------/----------\
  //                               /-------------------\
  //     <or-expr>  ::= <and-expr> {<or-op> <and-expr>}+    ----+
  //                    \--------/  \-----/                     |
  //                        /      \---\---------------/        |
  //                       /            \     \                 |
  //                      /              \     \                |
  //                    LHS               \     -- RHS          |
  //                                       --- oper_group       |
  //                                                            |
  //                    | <and-expr>   <------------------------+  (recursive
  //                      \--------/                             base case when
  //                          /                                     <term>)
  //                         /
  //                       LHS

  std::function<AstNodeRef(const OperationGroup_t&)> parse_subexpression;

  parse_subexpression =
      [&](const OperationGroup_t& subexpression_oper_group) -> AstNodeRef {
    std::function<AstNodeRef(AstNodeRef, const OperationGroup_t&)>
        parse_oper_chain;

    parse_oper_chain = [&](AstNodeRef lhs,
                           const OperationGroup_t& oper_group) -> AstNodeRef {
      // operators_in_group - current subexpression's and parse_oper_chain's
      // equal-precedence operators
      auto& operators_in_group = OperationOpMap[oper_group];

      // if current_token's operation isn't part of our operation group, then
      // return
      if (!operators_in_group.contains(current_token.get().value_enum))
      {
        return lhs;
      }

      // consume the operator token at this point so that on the recursive
      // return, it will become the root node of the parsed subexpression.
      const auto& operator_token = current_token.get().value_enum;
      get_next_token();

      std::optional<AstNodeRef> rhs;

      // parse_oper_chain() recursive base case
      if (oper_group == OperationGroup_t::P_MUL)
      {
        rhs = parse_subexpression(OperationGroup_t::P_MUL);
      }
      else
      {
        const OperationGroup_t next_expression_oper_group =
            next_in_oper_group(oper_group);

        rhs = parse_subexpression(next_expression_oper_group);

        if (rhs.value().get() == EmptyNodeRef.get())
        {
          return lhs;
        }
      }

      std::optional<AstNodeType_t> subexpression_node_type;

      // TODO: convert a mapping
      if (operator_token == TokenValue_t::J_DIVIDE)
        subexpression_node_type = AstNodeType_t::N_OP_DIVIDE;
      else if (operator_token == TokenValue_t::J_ASTERISK)
        subexpression_node_type = AstNodeType_t::N_OP_MULTIPLY;
      else if (operator_token == TokenValue_t::J_MINUS)
        subexpression_node_type = AstNodeType_t::N_OP_SUBTRACT;
      else if (operator_token == TokenValue_t::J_PLUS)
        subexpression_node_type = AstNodeType_t::N_OP_ADD;
      else if (operator_token == TokenValue_t::J_EQUAL)
        subexpression_node_type = AstNodeType_t::N_OP_LOGICAL_EQUALS;
      else if (operator_token == TokenValue_t::J_GREATER_THAN)
        subexpression_node_type = AstNodeType_t::N_OP_LOGICAL_GT;
      else if (operator_token == TokenValue_t::J_LESS_THAN)
        subexpression_node_type = AstNodeType_t::N_OP_LOGICAL_LT;
      else if (operator_token == TokenValue_t::J_AMPERSAND)
        subexpression_node_type = AstNodeType_t::N_OP_LOGICAL_AND;
      else if (operator_token == TokenValue_t::J_VBAR)
        subexpression_node_type = AstNodeType_t::N_OP_LOGICAL_OR;

      AstNodeRef subexpression_tree =
          create_ast_node(subexpression_node_type.value());

      subexpression_tree.get().add_child(lhs);
      subexpression_tree.get().add_child(rhs.value());

      return subexpression_tree;
    };  // parse_oper_chain() lambda

    //
    // parse_subexpression() starts here
    //

    std::optional<AstNodeRef> lhs;

    // parse_subexpression() recursive base case
    if (subexpression_oper_group == OperationGroup_t::P_MUL)
    {
      lhs = parse_term();
    }
    else
    {
      const OperationGroup_t next_subexpression_oper =
          next_in_oper_group(subexpression_oper_group);

      lhs = parse_subexpression(next_subexpression_oper);
    }

    AstNodeRef subexpression_tree =
        parse_oper_chain(lhs.value(), subexpression_oper_group);

    return subexpression_tree;
  };  // parse_subexpression() lambda

  //
  // parse_expression() starts here
  //

  // lowest operation precedence group is P_OR
  AstNodeRef ExpressionAst = parse_subexpression(OperationGroup_t::P_OR);

  return ExpressionAst;
}

AstNodeRef Parser::parse_term()
{
  AstNodeRef TermAst = EmptyNodeRef;

  // TODO: check old parser here.  can use value better

  if (current_token.get().value_enum == TokenValue_t::J_INTEGER_CONSTANT)
  {
    TermAst = create_ast_node(AstNodeType_t::N_INTEGER_CONSTANT,
                              current_token.get().value_str);
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
    TermAst = create_ast_node(AstNodeType_t::N_KEYWORD_CONSTANT);
    TermAst.get().add_child(create_ast_node(AstNodeType_t::N_TRUE_KEYWORD));
    get_next_token();
  }
  else if (current_token.get().value_enum == TokenValue_t::J_FALSE)
  {
    TermAst = create_ast_node(AstNodeType_t::N_KEYWORD_CONSTANT);
    TermAst.get().add_child(create_ast_node(AstNodeType_t::N_FALSE_KEYWORD));
    get_next_token();
  }
  else if (current_token.get().value_enum == TokenValue_t::J_NULL)
  {
    TermAst = create_ast_node(AstNodeType_t::N_KEYWORD_CONSTANT);
    TermAst.get().add_child(create_ast_node(AstNodeType_t::N_NULL_KEYWORD));
    get_next_token();
  }
  else if (current_token.get().value_enum == TokenValue_t::J_THIS)
  {
    TermAst = create_ast_node(AstNodeType_t::N_KEYWORD_CONSTANT);
    TermAst.get().add_child(create_ast_node(AstNodeType_t::N_THIS_KEYWORD));
    get_next_token();
  }
  else if (current_token.get().value_enum == TokenValue_t::J_LEFT_PARENTHESIS)
  {
    get_next_token();
    TermAst = parse_expression();
    require_token(current_token.get().value_enum,
                  TokenValue_t::J_RIGHT_PARENTHESIS);
    get_next_token();
  }

  if ((TermAst.get().type == EmptyNodeRef.get().type) &&
      (current_token.get().value_enum == TokenValue_t::J_IDENTIFIER))
  {
    TermAst = parse_subroutine_call();
  }

  if ((TermAst.get().type == EmptyNodeRef.get().type) &&
      (current_token.get().value_enum == TokenValue_t::J_IDENTIFIER))
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

  // TODO: improve
  if (TermAst.get().type == EmptyNodeRef.get().type)
  {
    std::stringstream ss;

    ss << "(TODO) Line #" << current_token.get().line_number << ": "
       << "Expected TERM token while parsing "
       << JackToken::to_string(current_token.get().value_enum);

#ifndef WIN32
    raise(SIGTRAP);
#endif
    fatal_error(ss.str());
  }

  return TermAst;
}

}  // namespace recursive_descent
