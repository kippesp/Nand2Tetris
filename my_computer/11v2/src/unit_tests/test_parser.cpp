#include <iostream>

#include "common.h"
#include "jack_sources.h"

#if 0
extern const char* STRING_TERM_SRC;
extern const char* TWO_SUBS_SRC;
extern const char* OBJECT_METHOD_CALL_SRC;
extern const char* SIMPLE_IF_SRC;
extern const char* LHS_ARRAY_ASSIGN_SRC;
extern const char* RHS_ARRAY_ASSIGN_SRC;
extern const char* ARRAY_ARRAY_ASSIGN_SRC;
extern const char* NUMERICAL_IF_SRC;
extern const char* JACK_SEVEN_SRC;
extern const char* LET_STATEMENT_SRC;
extern const char* SIMPLE_WHILE_SRC;
#endif

SCENARIO("Parse tree simple terms")
{
  SECTION("simple let")
  {
    TextReader R("let i = true;");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_let_statement();

    std::string as_str = root.get().as_s_expression();

    Expected_t expected = {
        ""  // clang-format sorcery
        "(LET_STATEMENT",
        "  (SCALAR_VAR",
        "    (VARIABLE_NAME string_value:i))",
        "  (EXPRESSION",
        "    (TERM",
        "      (KEYWORD_CONSTANT",
        "        (TRUE_KEYWORD)))))"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }

  SECTION("single return function")
  {
    TextReader R(SINGLE_RETURN_SRC);
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_class();
    std::string as_str = root.get().as_s_expression();

    Expected_t expected = {
        ""
        "(CLASS_DECL string_value:Main",
        "  (FUNCTION_DECL string_value:f1",
        "    (SUBROUTINE_DESCR",
        "      (SUBROUTINE_DECL_RETURN_TYPE string_value:int))",
        "    (SUBROUTINE_BODY",
        "      (STATEMENT_BLOCK",
        "        (RETURN_STATEMENT",
        "          (EXPRESSION",
        "            (TERM",
        "              (INTEGER_CONSTANT string_value:1))))))))"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }

  SECTION("void return method")
  {
    TextReader R(VOID_RETURN_SRC);
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_class();
    std::string as_str = root.get().as_s_expression();

    Expected_t expected = {
        ""
        "(CLASS_DECL string_value:Test",
        "  (METHOD_DECL string_value:f1",
        "    (SUBROUTINE_DESCR",
        "      (SUBROUTINE_DECL_RETURN_TYPE string_value:void))",
        "    (SUBROUTINE_BODY",
        "      (STATEMENT_BLOCK",
        "        (RETURN_STATEMENT)))))"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }

  SECTION("simple constructor method")
  {
    TextReader R(SIMPLE_CONST_SRC);
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_class();
    std::string as_str = root.get().as_s_expression();

    Expected_t expected = {
        ""
        "(CLASS_DECL string_value:Test",
        "  (CONSTRUCTOR_DECL string_value:new",
        "    (SUBROUTINE_DESCR",
        "      (SUBROUTINE_DECL_RETURN_TYPE string_value:Test))",
        "    (SUBROUTINE_BODY",
        "      (STATEMENT_BLOCK",
        "        (RETURN_STATEMENT",
        "          (EXPRESSION",
        "            (TERM",
        "              (KEYWORD_CONSTANT",
        "                (THIS_KEYWORD))))))))",
        "  (METHOD_DECL string_value:ref",
        "    (SUBROUTINE_DESCR",
        "      (SUBROUTINE_DECL_RETURN_TYPE string_value:int))",
        "    (SUBROUTINE_BODY",
        "      (STATEMENT_BLOCK",
        "        (RETURN_STATEMENT",
        "          (EXPRESSION",
        "            (TERM",
        "              (KEYWORD_CONSTANT",
        "                (THIS_KEYWORD)))))))))"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }

  SECTION("class vars")
  {
    TextReader R(CLASSVAR_SRC);
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_class();
    std::string as_str = root.get().as_s_expression();

    Expected_t expected = {
        ""
        "(CLASS_DECL string_value:testjack",
        "  (CLASSVAR_DECL_BLOCK",
        "    (CLASSVAR_DECL",
        "      (VARIABLE_NAME string_value:inst1)",
        "      (CLASSVAR_SCOPE",
        "        (STATIC_SCOPE))",
        "      (VARIABLE_TYPE",
        "        (CLASS_TYPE string_value:ClassName)))",
        "    (CLASSVAR_DECL",
        "      (VARIABLE_NAME string_value:var1)",
        "      (CLASSVAR_SCOPE",
        "        (FIELD_SCOPE))",
        "      (VARIABLE_TYPE",
        "        (INTEGER_TYPE)))",
        "    (CLASSVAR_DECL",
        "      (VARIABLE_NAME string_value:var2)",
        "      (CLASSVAR_SCOPE",
        "        (FIELD_SCOPE))",
        "      (VARIABLE_TYPE",
        "        (BOOLEAN_TYPE)))",
        "    (CLASSVAR_DECL",
        "      (VARIABLE_NAME string_value:var3)",
        "      (CLASSVAR_SCOPE",
        "        (STATIC_SCOPE))",
        "      (VARIABLE_TYPE",
        "        (CHAR_TYPE)))",
        "    (CLASSVAR_DECL",
        "      (VARIABLE_NAME string_value:var4)",
        "      (CLASSVAR_SCOPE",
        "        (STATIC_SCOPE))",
        "      (VARIABLE_TYPE",
        "        (CHAR_TYPE)))))"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }

#if 0
  SECTION("class and subroutine vars")
  {
    TextReader R(CLASSANDSUBVARS_SRC);
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    for (const auto& token : tokens)
    {
      if (token.type == TokenType_t::J_INTERNAL)
      {
        continue;
      }

      std::cout << token.to_s_expression() << std::endl;
    }
#if 0
(KEYWORD CLASS class)
(IDENTIFIER NON_ENUM JackTest)
(SYMBOL LEFT_BRACE <left_brace>)
(KEYWORD STATIC static)
(IDENTIFIER NON_ENUM ClassName)
(IDENTIFIER NON_ENUM inst1)
(SYMBOL SEMICOLON <semicolon>)
(KEYWORD FIELD field)
(KEYWORD INT integer)
(IDENTIFIER NON_ENUM var1)
(SYMBOL SEMICOLON <semicolon>)
(KEYWORD FIELD field)
(KEYWORD BOOLEAN boolean)
(IDENTIFIER NON_ENUM var2)
(SYMBOL SEMICOLON <semicolon>)
(KEYWORD METHOD method)
(KEYWORD VOID void)
(IDENTIFIER NON_ENUM sub1)
(SYMBOL LEFT_PARENTHESIS <left_parenthesis>)
(KEYWORD INT integer)
(IDENTIFIER NON_ENUM a1)
(SYMBOL COMMA <comma>)
(KEYWORD INT integer)
(IDENTIFIER NON_ENUM a2)
(SYMBOL RIGHT_PARENTHESIS <right_parenthesis>)
(SYMBOL LEFT_BRACE <left_brace>)
(KEYWORD VAR var)
(KEYWORD INT integer)
(IDENTIFIER NON_ENUM v1)
(SYMBOL SEMICOLON <semicolon>)
(KEYWORD VAR var)
(KEYWORD BOOLEAN boolean)
(IDENTIFIER NON_ENUM v2)
(SYMBOL COMMA <comma>)
(IDENTIFIER NON_ENUM v3)
(SYMBOL SEMICOLON <semicolon>)
(KEYWORD RETURN return)
(SYMBOL SEMICOLON <semicolon>)
(SYMBOL RIGHT_BRACE <right_brace>)
(SYMBOL RIGHT_BRACE <right_brace>)
#endif

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_class();
    std::string as_str = root.get().as_s_expression();
    REQUIRE(as_str ==
            "(CLASS_DECL string_value:Main\n"
            "  (FUNCTION_DECL string_value:f1\n"
            "    (SUBROUTINE_DESCR\n"
            "      (SUBROUTINE_DECL_RETURN_TYPE string_value:int))\n"
            "    (SUBROUTINE_BODY\n"
            "      (STATEMENT_BLOCK\n"
            "        (RETURN_STATEMENT\n"
            "          (EXPRESSION\n"
            "            (TERM\n"
            "              (INTEGER_CONSTANT string_value:1))))))))");
  }
#endif

#if 0
  SECTION("constant void method call")
  {
    TextReader R(CONST_VOID_METHOD_CALL_SRC);
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    std::cout << CONST_VOID_METHOD_CALL_SRC << std::endl;

    for (const auto& token : tokens)
    {
      if (token.type == TokenType_t::J_INTERNAL)
      {
        continue;
      }

      std::cout << token.to_s_expression() << std::endl;
    }
#if 0
(KEYWORD CLASS class)
(IDENTIFIER NON_ENUM Test)
(SYMBOL LEFT_BRACE <left_brace>)
(KEYWORD FIELD field)
(KEYWORD INT integer)
(IDENTIFIER NON_ENUM a)
(SYMBOL SEMICOLON <semicolon>)
(KEYWORD CONSTRUCTOR constructor)
(IDENTIFIER NON_ENUM Test)
(IDENTIFIER NON_ENUM new)
(SYMBOL LEFT_PARENTHESIS <left_parenthesis>)
(SYMBOL RIGHT_PARENTHESIS <right_parenthesis>)
(SYMBOL LEFT_BRACE <left_brace>)
(KEYWORD DO do)
(IDENTIFIER NON_ENUM draw)
(SYMBOL LEFT_PARENTHESIS <left_parenthesis>)
(SYMBOL RIGHT_PARENTHESIS <right_parenthesis>)
(SYMBOL SEMICOLON <semicolon>)
(KEYWORD RETURN return)
(KEYWORD THIS this)
(SYMBOL SEMICOLON <semicolon>)
(SYMBOL RIGHT_BRACE <right_brace>)
(KEYWORD METHOD method)
(KEYWORD VOID void)
(IDENTIFIER NON_ENUM draw)
(SYMBOL LEFT_PARENTHESIS <left_parenthesis>)
(SYMBOL RIGHT_PARENTHESIS <right_parenthesis>)
(SYMBOL LEFT_BRACE <left_brace>)
(KEYWORD RETURN return)
(SYMBOL SEMICOLON <semicolon>)
(SYMBOL RIGHT_BRACE <right_brace>)
(SYMBOL RIGHT_BRACE <right_brace>)
#endif

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_class();
    std::string as_str = root.get().as_s_expression();
    REQUIRE(as_str ==
            "(CLASS_DECL string_value:Main\n"
            "  (FUNCTION_DECL string_value:f1\n"
            "    (SUBROUTINE_DESCR\n"
            "      (SUBROUTINE_DECL_RETURN_TYPE string_value:int))\n"
            "    (SUBROUTINE_BODY\n"
            "      (STATEMENT_BLOCK\n"
            "        (RETURN_STATEMENT\n"
            "          (EXPRESSION\n"
            "            (TERM\n"
            "              (INTEGER_CONSTANT string_value:1))))))))");
  }
#endif

#if 0
  SECTION("class method call")
  {
    TextReader R(CLASS_METHOD_CALL_SRC);
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    for (const auto& token : tokens)
    {
      if (token.type == TokenType_t::J_INTERNAL)
      {
        continue;
      }

      std::cout << token.to_s_expression() << std::endl;
    }
#if 0
(KEYWORD CLASS class)
(IDENTIFIER NON_ENUM Main)
(SYMBOL LEFT_BRACE <left_brace>)
(KEYWORD METHOD method)
(KEYWORD INT integer)
(IDENTIFIER NON_ENUM mul)
(SYMBOL LEFT_PARENTHESIS <left_parenthesis>)
(KEYWORD INT integer)
(IDENTIFIER NON_ENUM a)
(SYMBOL COMMA <comma>)
(IDENTIFIER NON_ENUM b)
(SYMBOL RIGHT_PARENTHESIS <right_parenthesis>)
(SYMBOL LEFT_BRACE <left_brace>)
(KEYWORD RETURN return)
(INTEGER_CONSTANT NON_ENUM 0)
(SYMBOL SEMICOLON <semicolon>)
(SYMBOL RIGHT_BRACE <right_brace>)
(KEYWORD METHOD method)
(KEYWORD INT integer)
(IDENTIFIER NON_ENUM f1)
(SYMBOL LEFT_PARENTHESIS <left_parenthesis>)
(SYMBOL RIGHT_PARENTHESIS <right_parenthesis>)
(SYMBOL LEFT_BRACE <left_brace>)
(KEYWORD RETURN return)
(IDENTIFIER NON_ENUM mul)
(SYMBOL LEFT_PARENTHESIS <left_parenthesis>)
(INTEGER_CONSTANT NON_ENUM 1)
(SYMBOL COMMA <comma>)
(INTEGER_CONSTANT NON_ENUM 2)
(SYMBOL RIGHT_PARENTHESIS <right_parenthesis>)
(SYMBOL SEMICOLON <semicolon>)
(SYMBOL RIGHT_BRACE <right_brace>)
(SYMBOL RIGHT_BRACE <right_brace>)
#endif

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_class();
    std::string as_str = root.get().as_s_expression();
    REQUIRE(as_str ==
            "(CLASS_DECL string_value:Main\n"
            "  (FUNCTION_DECL string_value:f1\n"
            "    (SUBROUTINE_DESCR\n"
            "      (SUBROUTINE_DECL_RETURN_TYPE string_value:int))\n"
            "    (SUBROUTINE_BODY\n"
            "      (STATEMENT_BLOCK\n"
            "        (RETURN_STATEMENT\n"
            "          (EXPRESSION\n"
            "            (TERM\n"
            "              (INTEGER_CONSTANT string_value:1))))))))");
  }
#endif
}
