#include "parser.h"

#include <cassert>
#include <list>
#include <optional>

#include <signal.h>

using namespace ast;

namespace recursive_descent {

Parser::Parser(JackTokenizer::Tokens_t& tokens)
    : token_iter(tokens.begin()),
      token_iter_end(tokens.end()),
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

    raise(SIGTRAP);
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

    raise(SIGTRAP);
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
  require_token(start_token, TokenType_t::J_IDENTIFIER);

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
      require_token(start_token, TokenType_t::J_IDENTIFIER);
      VarTypeAst = create_ast_node(AstNodeType_t::N_VARIABLE_CLASS_TYPE,
                                   current_token.get().value_str);
    }

    auto add_def_to_block = [&]() {
      require_token(start_token, TokenType_t::J_IDENTIFIER);

      AstNodeRef ClassVarDecl = create_ast_node(class_var_scope);

      AstNodeRef ClassVarDeclName = create_ast_node(
          AstNodeType_t::N_VARIABLE_NAME, current_token.get().value_str);

      ClassVarDecl.get().add_child(ClassVarDeclName);
      ClassVarDecl.get().add_child(VarTypeAst.value());

      ClassVarDeclBlockAst.get().add_child(ClassVarDecl);
      get_next_token();
    };

    get_next_token();
    require_token(start_token, TokenType_t::J_IDENTIFIER);
    add_def_to_block();

    while (current_token.get().value_enum == TokenValue_t::J_COMMA)
    {
      get_next_token();
      require_token(start_token, TokenType_t::J_IDENTIFIER);
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
           << JackToken::to_string(TokenValue_t::J_CONSTRUCTOR  )
           << " while parsing "
           << JackToken::to_string(TokenValue_t::J_CLASS);

        fatal_error(ss.str());
    }

    get_next_token();

    AstNodeRef SubrDescrAst =
        create_ast_node(AstNodeType_t::N_SUBROUTINE_DESCR);

    // <subroutine-decl>  ::= ("constructor" | "function" | "method")
    //                        ("void" | <type>) <subroutine-name>
    //                        \---------------/
    //                                /
    //                               /
    //                     SubrDeclReturnTypeAst

    std::optional<AstNodeRef> SubrDeclReturnTypeAst;

    if (current_token.get().type == TokenType_t::J_IDENTIFIER)
    {
      SubrDeclReturnTypeAst =
          create_ast_node(AstNodeType_t::N_SUBROUTINE_DECL_RETURN_TYPE,
                          current_token.get().value_str);
    }
    else if (current_token.get().type == TokenType_t::J_KEYWORD)
    {
      if (current_token.get().value_enum == TokenValue_t::J_INT)
      {
          SubrDeclReturnTypeAst = create_ast_node(
              AstNodeType_t::N_SUBROUTINE_DECL_RETURN_TYPE, "int");
      }
      else if (current_token.get().value_enum == TokenValue_t::J_CHAR)
      {
        SubrDeclReturnTypeAst = create_ast_node(
            AstNodeType_t::N_SUBROUTINE_DECL_RETURN_TYPE, "char");
      }
      else if (current_token.get().value_enum == TokenValue_t::J_BOOLEAN)
      {
        SubrDeclReturnTypeAst = create_ast_node(
            AstNodeType_t::N_SUBROUTINE_DECL_RETURN_TYPE, "boolean");
      }
      else if (current_token.get().value_enum == TokenValue_t::J_VOID)
      {
        SubrDeclReturnTypeAst = create_ast_node(
            AstNodeType_t::N_SUBROUTINE_DECL_RETURN_TYPE, "void");
      }
      else
      {
        std::stringstream ss;

        ss << "Line #" << current_token.get().line_number << ": "
           << "Expected int|char|boolean|void|ClassName"
           << " while parsing " << JackToken::to_string(start_token);

        fatal_error(ss.str());
      }
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
    require_token(start_token, TokenType_t::J_IDENTIFIER);

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
          require_token(start_token, TokenType_t::J_IDENTIFIER);
          VarTypeAst = create_ast_node(AstNodeType_t::N_VARIABLE_CLASS_TYPE,
                                       current_token.get().value_str);
        }

        get_next_token();
        require_token(start_token, TokenType_t::J_IDENTIFIER);

        AstNodeRef ParamDeclName = create_ast_node(
            AstNodeType_t::N_PARAMETER_DECL);

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
          VarTypeAst =
              create_ast_node(AstNodeType_t::N_VARIABLE_CHARACTER_TYPE);
        }
        else if (current_token.get().value_enum == TokenValue_t::J_BOOLEAN)
        {
          VarTypeAst = create_ast_node(AstNodeType_t::N_VARIABLE_BOOLEAN_TYPE);
        }
        else
        {
          require_token(start_token, TokenType_t::J_IDENTIFIER);
          VarTypeAst = create_ast_node(AstNodeType_t::N_VARIABLE_CLASS_TYPE,
                                       current_token.get().value_str);
        }

        do
        {
          get_next_token();
          require_token(start_token, TokenType_t::J_IDENTIFIER);

          AstNodeRef SubroutineVarDeclAst =
              create_ast_node(AstNodeType_t::N_LOCAL_VAR_DECL);

          AstNodeRef VarNameAst = create_ast_node(
              AstNodeType_t::N_VARIABLE_NAME, current_token.get().value_str);

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
  get_next_token();

  return LetAst;
}

ast::AstNodeRef Parser::parse_return_statement()
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
  else if (current_token.get().type == TokenType_t::J_INTEGER_CONSTANT)
  {
    AstNodeRef TermAst = create_ast_node(AstNodeType_t::N_TERM);
    AstNodeRef IntegerConstAst = create_ast_node(
        AstNodeType_t::N_INTEGER_CONSTANT, current_token.get().value_str);

    TermAst.get().add_child(IntegerConstAst);

    ExpressionAst.get().add_child(TermAst);

    get_next_token();
  }
  else if (current_token.get().value_enum == TokenValue_t::J_THIS)
  {
    AstNodeRef TermAst = create_ast_node(AstNodeType_t::N_TERM);
    AstNodeRef KeywordConstAst =
        create_ast_node(AstNodeType_t::N_KEYWORD_CONSTANT);
    AstNodeRef TrueKeywordAst = create_ast_node(AstNodeType_t::N_THIS_KEYWORD);

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
