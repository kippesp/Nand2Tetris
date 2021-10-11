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
      EmptyNodeRef(*(std::make_unique<ast::AstNode>(
          ast::AstNode {ast::AstNodeType_t::N_UNDEFINED})))
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
    AstNodeRef ClassVarDeclBlockAst = parse_classvar_decl_block();

    ClassAst.get().add_child(ClassVarDeclBlockAst);
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

  AstNodeRef ClassVarDeclBlockAst =
      create_ast_node(AstNodeType_t::N_CLASSVAR_DECL_BLOCK);

  while ((current_token.get().value_enum == TokenValue_t::J_STATIC) ||
         (current_token.get().value_enum == TokenValue_t::J_FIELD))
  {
    AstNodeType_t class_var_scope = AstNodeType_t::N_UNDEFINED;

    if (current_token.get().value_enum == TokenValue_t::J_STATIC)
    {
      class_var_scope = AstNodeType_t::N_CLASSVAR_STATIC_DECL;
    }
    else if (current_token.get().value_enum == TokenValue_t::J_FIELD)
    {
      class_var_scope = AstNodeType_t::N_CLASSVAR_FIELD_DECL;
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

    std::optional<AstNodeRef> VarTypeAst;

    if (current_token.get().value_enum == TokenValue_t::J_INT)
    {
      VarTypeAst = create_ast_node(AstNodeType_t::N_VARIABLE_INTEGER_TYPE);
    }
    else if (current_token.get().value_enum == TokenValue_t::J_CHAR)
    {
      VarTypeAst = create_ast_node(AstNodeType_t::N_VARIABLE_CHARACTER_TYPE);
    }
    else if (current_token.get().value_enum == TokenValue_t::J_BOOLEAN)
    {
      VarTypeAst = create_ast_node(AstNodeType_t::N_VARIABLE_BOOLEAN_TYPE);
    }
    else
    {
      require_token(start_token, TokenValue_t::J_IDENTIFIER);
      VarTypeAst = create_ast_node(AstNodeType_t::N_VARIABLE_CLASS_TYPE,
                                   current_token.get().value_str);
    }

    auto add_def_to_block = [&]() {
      require_token(start_token, TokenValue_t::J_IDENTIFIER);

      AstNodeRef ClassVarDecl = create_ast_node(class_var_scope);

      AstNodeRef ClassVarDeclName = create_ast_node(
          AstNodeType_t::N_VARIABLE_NAME, current_token.get().value_str);

      ClassVarDecl.get().add_child(ClassVarDeclName);
      ClassVarDecl.get().add_child(VarTypeAst.value());

      ClassVarDeclBlockAst.get().add_child(ClassVarDecl);
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

  return ClassVarDeclBlockAst;
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
    SubrDeclReturnTypeAst =
        create_ast_node(AstNodeType_t::N_SUBROUTINE_DECL_RETURN_TYPE,
                        current_token.get().value_str);
  }
  else if (current_token.get().value_enum == TokenValue_t::J_INT)
  {
    SubrDeclReturnTypeAst =
        create_ast_node(AstNodeType_t::N_SUBROUTINE_DECL_RETURN_TYPE, "int");
  }
  else if (current_token.get().value_enum == TokenValue_t::J_CHAR)
  {
    SubrDeclReturnTypeAst =
        create_ast_node(AstNodeType_t::N_SUBROUTINE_DECL_RETURN_TYPE, "char");
  }
  else if (current_token.get().value_enum == TokenValue_t::J_BOOLEAN)
  {
    SubrDeclReturnTypeAst = create_ast_node(
        AstNodeType_t::N_SUBROUTINE_DECL_RETURN_TYPE, "boolean");
  }
  else if (current_token.get().value_enum == TokenValue_t::J_VOID)
  {
    SubrDeclReturnTypeAst =
        create_ast_node(AstNodeType_t::N_SUBROUTINE_DECL_RETURN_TYPE, "void");
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
  //                        "(" <parameter-list> ")" <subroutine-body>
  //                            \--------------/
  //                                    /
  //                                   /
  //          +-----------------------+
  //         /
  //        /
  // /--------------\
  // <parameter-list>   ::= {(<type> <var-name>) {"," <type> <var-name>}*}?
  //       \
  //        -- N_PARAMETER_LIST

  if (current_token.get().value_enum != TokenValue_t::J_RIGHT_PARENTHESIS)
  {
    AstNodeRef ParamListAst = create_ast_node(AstNodeType_t::N_PARAMETER_LIST);

    while (current_token.get().value_enum != TokenValue_t::J_RIGHT_PARENTHESIS)
    {
      std::optional<AstNodeRef> VarTypeAst;

      // TODO: pattern is very common
      if (current_token.get().value_enum == TokenValue_t::J_INT)
      {
        VarTypeAst = create_ast_node(AstNodeType_t::N_VARIABLE_INTEGER_TYPE);
      }
      else if (current_token.get().value_enum == TokenValue_t::J_CHAR)
      {
        VarTypeAst = create_ast_node(AstNodeType_t::N_VARIABLE_CHARACTER_TYPE);
      }
      else if (current_token.get().value_enum == TokenValue_t::J_BOOLEAN)
      {
        VarTypeAst = create_ast_node(AstNodeType_t::N_VARIABLE_BOOLEAN_TYPE);
      }
      else
      {
        require_token(start_token, TokenValue_t::J_IDENTIFIER);
        VarTypeAst = create_ast_node(AstNodeType_t::N_VARIABLE_CLASS_TYPE,
                                     current_token.get().value_str);
      }

      get_next_token();
      require_token(start_token, TokenValue_t::J_IDENTIFIER);

      AstNodeRef ParamDeclName =
          create_ast_node(AstNodeType_t::N_PARAMETER_DECL);

      ParamDeclName.get().add_child(create_ast_node(
          AstNodeType_t::N_VARIABLE_NAME, current_token.get().value_str));
      ParamDeclName.get().add_child(VarTypeAst.value());

      ParamListAst.get().add_child(ParamDeclName);

      get_next_token();

      if (current_token.get().value_enum == TokenValue_t::J_COMMA)
      {
        get_next_token();
      }
    }

    SubrDescrAst.get().add_child(ParamListAst);
  }

  require_token(start_token, TokenValue_t::J_RIGHT_PARENTHESIS);
  get_next_token();

  require_token(start_token, TokenValue_t::J_LEFT_BRACE);
  get_next_token();

  //
  // <subroutine-body>  ::= "{" {<var-decl>}* {<statement>}* "}"
  //                            \----------/
  //                                 / \
  //                                /   ------- N_LOCAL_VAR_DECL_BLOCK
  //       +-----------------------+
  //      /                                 ------ N_LOCAL_VAR_DECL
  //     /                                 /
  // /--------\                   /---------------\
  // <var-decl>         ::= "var" <type> <var-name> {"," <var-name>}* ";"
  //                              \----/ \--------/
  //                                /         \
  //         N_VARIABLE_XX_TYPE ----           ---- N_VARIABLE_NAME

  AstNodeRef SubroutineBodyAst =
      create_ast_node(AstNodeType_t::N_SUBROUTINE_BODY);

  if (current_token.get().value_enum == TokenValue_t::J_VAR)
  {
    AstNodeRef SubroutineVarDeclBlockAst =
        create_ast_node(AstNodeType_t::N_LOCAL_VAR_DECL_BLOCK);

    while (current_token.get().value_enum == TokenValue_t::J_VAR)
    {
      std::optional<AstNodeRef> VarTypeAst;

      get_next_token();

      if (current_token.get().value_enum == TokenValue_t::J_INT)
      {
        VarTypeAst = create_ast_node(AstNodeType_t::N_VARIABLE_INTEGER_TYPE);
      }
      else if (current_token.get().value_enum == TokenValue_t::J_CHAR)
      {
        VarTypeAst = create_ast_node(AstNodeType_t::N_VARIABLE_CHARACTER_TYPE);
      }
      else if (current_token.get().value_enum == TokenValue_t::J_BOOLEAN)
      {
        VarTypeAst = create_ast_node(AstNodeType_t::N_VARIABLE_BOOLEAN_TYPE);
      }
      else
      {
        require_token(start_token, TokenValue_t::J_IDENTIFIER);
        VarTypeAst = create_ast_node(AstNodeType_t::N_VARIABLE_CLASS_TYPE,
                                     current_token.get().value_str);
      }

      do
      {
        get_next_token();
        require_token(start_token, TokenValue_t::J_IDENTIFIER);

        AstNodeRef SubroutineVarDeclAst =
            create_ast_node(AstNodeType_t::N_LOCAL_VAR_DECL);

        AstNodeRef VarNameAst = create_ast_node(AstNodeType_t::N_VARIABLE_NAME,
                                                current_token.get().value_str);

        SubroutineVarDeclAst.get().add_child(VarNameAst);
        SubroutineVarDeclAst.get().add_child(VarTypeAst.value());

        SubroutineVarDeclBlockAst.get().add_child(SubroutineVarDeclAst);
        get_next_token();
      } while (current_token.get().value_enum == TokenValue_t::J_COMMA);

      require_token(start_token, TokenValue_t::J_SEMICOLON);
      get_next_token();
    }

    SubroutineBodyAst.get().add_child(SubroutineVarDeclBlockAst);
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

    while (current_token.get().value_enum != TokenValue_t::J_RIGHT_BRACE)
    {
      if (current_token.get().value_enum == TokenValue_t::J_LET)
      {
        AstNodeRef LetAst = parse_let_statement();
        StatementBlockAst.get().add_child(LetAst);
      }
      else if (current_token.get().value_enum == TokenValue_t::J_DO)
      {
      }
      else if (current_token.get().value_enum == TokenValue_t::J_RETURN)
      {
        AstNodeRef ReturnAst = parse_return_statement();
        StatementBlockAst.get().add_child(ReturnAst);
      }
      else
      {
        assert(0 && "statement required");
      }
    }

    SubroutineBodyAst.get().add_child(StatementBlockAst);
  }

  SubrDeclAst.value().get().add_child(SubroutineBodyAst);

  require_token(start_token, TokenValue_t::J_RIGHT_BRACE);
  get_next_token();

  return SubrDeclAst.value();
}

AstNodeRef Parser::parse_let_statement()
{
  const auto start_token = TokenValue_t::J_LET;
  assert(current_token.get().value_enum == start_token);

  AstNodeRef LetAst = create_ast_node(AstNodeType_t::N_LET_STATEMENT);
  get_next_token();

  require_token(start_token, TokenValue_t::J_IDENTIFIER);

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
  get_next_token();

  return LetAst;
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

  // root of an expression is (N_EXPRESSION) node
  AstNodeRef ExpressionAst = create_ast_node(AstNodeType_t::N_EXPRESSION);

  // lowest operation precedence group is P_OR
  AstNodeRef OrExpr = parse_subexpression(OperationGroup_t::P_OR);

  if (OrExpr.get() != EmptyNodeRef.get())
  {
    ExpressionAst.get().add_child(OrExpr);
  }

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

  // TODO: improve
  if (TermAst.get().type == EmptyNodeRef.get().type)
  {
    std::stringstream ss;

    ss << "(TODO) Line #" << current_token.get().line_number << ": "
       << "Expected TERM token while parsing "
       << JackToken::to_string(current_token.get().value_enum);

#ifndef WIN32
    // raise(SIGTRAP);
#endif
    fatal_error(ss.str());
  }

  return TermAst;
}

}  // namespace recursive_descent
