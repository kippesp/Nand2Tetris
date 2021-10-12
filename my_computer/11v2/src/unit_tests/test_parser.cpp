#include <iostream>

#include "common.h"
#include "jack_sources.h"

#if 0
extern const char* CONST_VOID_METHOD_CALL_SRC;
extern const char* JACK_SEVEN_SRC;

extern const char* STRING_TERM_SRC;
extern const char* CLASS_METHOD_CALL_SRC);
extern const char* OBJECT_METHOD_CALL_SRC;
extern const char* SIMPLE_IF_SRC;
extern const char* TWO_SUBS_SRC;

extern const char* NUMERICAL_IF_SRC;
extern const char* LET_STATEMENT_SRC;
extern const char* SIMPLE_WHILE_SRC;
extern const char* LHS_ARRAY_ASSIGN_SRC;
extern const char* RHS_ARRAY_ASSIGN_SRC;
extern const char* ARRAY_ARRAY_ASSIGN_SRC;
#endif

SCENARIO("Parse expressions")
{
  SECTION("single term")
  {
    TextReader R("1");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_expression();
    std::string as_str = root.get().as_s_expression();

    Expected_t expected = {
        ""  // clang-format sorcery
        "(INTEGER_CONSTANT string_value:1)"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }

  SECTION("single paren term")
  {
    TextReader R("((((1))))");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_expression();
    std::string as_str = root.get().as_s_expression();

    Expected_t expected = {
        ""  // clang-format sorcery
        "(INTEGER_CONSTANT string_value:1)"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }

  SECTION("simple multiply")
  {
    TextReader R("1 * 2");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_expression();
    std::string as_str = root.get().as_s_expression();

    Expected_t expected = {
        ""  // clang-format sorcery
        "(OP_MULTIPLY",
        "  (INTEGER_CONSTANT string_value:1)",
        "  (INTEGER_CONSTANT string_value:2))"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }

  SECTION("multiply chain")
  {
    TextReader R("1 * 2 / 3");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_expression();
    std::string as_str = root.get().as_s_expression();

    Expected_t expected = {
        ""  // clang-format sorcery
        "(OP_MULTIPLY",
        "  (INTEGER_CONSTANT string_value:1)",
        "  (OP_DIVIDE",
        "    (INTEGER_CONSTANT string_value:2)",
        "    (INTEGER_CONSTANT string_value:3)))"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }

  SECTION("simple string")
  {
    TextReader R("\"a string\"");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_expression();
    std::string as_str = root.get().as_s_expression();

    Expected_t expected = {
        ""  // clang-format sorcery
        "(STRING_CONSTANT string_value:a string)"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }
}

SCENARIO("Subroutine calls")
{
  SECTION("local call, no parms")
  {
    TextReader R("fn()");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_subroutine_call();
    std::string as_str = root.get().as_s_expression();

    Expected_t expected = {
        ""  // clang-format sorcery
        "(SUBROUTINE_CALL",
        "  (LOCAL_CALL_SITE",
        "    (SUBROUTINE_NAME string_value:fn)))"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }

  SECTION("global call, no parm")
  {
    TextReader R("MyClass.fn()");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_subroutine_call();
    std::string as_str = root.get().as_s_expression();

    Expected_t expected = {
        ""  // clang-format sorcery
        "(SUBROUTINE_CALL",
        "  (GLOBAL_CALL_SITE",
        "    (GLOBAL_BIND_NAME string_value:MyClass)",
        "    (SUBROUTINE_NAME string_value:fn)))"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }

  SECTION("local call, 1 parm")
  {
    TextReader R("fn(1)");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_subroutine_call();
    std::string as_str = root.get().as_s_expression();

    Expected_t expected = {
        ""  // clang-format sorcery
        "(SUBROUTINE_CALL",
        "  (LOCAL_CALL_SITE",
        "    (SUBROUTINE_NAME string_value:fn))",
        "  (INTEGER_CONSTANT string_value:1))"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }

  SECTION("local call, 2 parms")
  {
    TextReader R("fn(1, 2)");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_subroutine_call();
    std::string as_str = root.get().as_s_expression();

    Expected_t expected = {
        ""  // clang-format sorcery
        "(SUBROUTINE_CALL",
        "  (LOCAL_CALL_SITE",
        "    (SUBROUTINE_NAME string_value:fn))",
        "  (INTEGER_CONSTANT string_value:1)",
        "  (INTEGER_CONSTANT string_value:2))"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }

  SECTION("local call, 2 expression parms")
  {
    TextReader R("fn(2*3, 4*5)");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_subroutine_call();
    std::string as_str = root.get().as_s_expression();

    Expected_t expected = {
        ""  // clang-format sorcery
        "(SUBROUTINE_CALL",
        "  (LOCAL_CALL_SITE",
        "    (SUBROUTINE_NAME string_value:fn))",
        "  (OP_MULTIPLY",
        "    (INTEGER_CONSTANT string_value:2)",
        "    (INTEGER_CONSTANT string_value:3))",
        "  (OP_MULTIPLY",
        "    (INTEGER_CONSTANT string_value:4)",
        "    (INTEGER_CONSTANT string_value:5)))"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }
}

SCENARIO("Parse tree simple terms")
{
  SECTION("simple let")
  {
    TextReader R("let i = true;");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_statement();
    std::string as_str = root.get().as_s_expression();

    Expected_t expected = {
        ""  // clang-format sorcery
        "(LET_STATEMENT",
        "  (SCALAR_VAR",
        "    (VARIABLE_NAME string_value:i))",
        "  (KEYWORD_CONSTANT",
        "    (TRUE_KEYWORD)))"};

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
        "          (INTEGER_CONSTANT string_value:1))))))"};

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
        "          (KEYWORD_CONSTANT",
        "            (THIS_KEYWORD))))))",
        "  (METHOD_DECL string_value:ref",
        "    (SUBROUTINE_DESCR",
        "      (SUBROUTINE_DECL_RETURN_TYPE string_value:int))",
        "    (SUBROUTINE_BODY",
        "      (STATEMENT_BLOCK",
        "        (RETURN_STATEMENT",
        "          (KEYWORD_CONSTANT",
        "            (THIS_KEYWORD)))))))"};

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
        "    (CLASSVAR_STATIC_DECL",
        "      (VARIABLE_NAME string_value:inst1)",
        "      (VARIABLE_CLASS_TYPE string_value:ClassName))",
        "    (CLASSVAR_FIELD_DECL",
        "      (VARIABLE_NAME string_value:var1)",
        "      (VARIABLE_INTEGER_TYPE))",
        "    (CLASSVAR_FIELD_DECL",
        "      (VARIABLE_NAME string_value:var2)",
        "      (VARIABLE_BOOLEAN_TYPE))",
        "    (CLASSVAR_STATIC_DECL",
        "      (VARIABLE_NAME string_value:var3)",
        "      (VARIABLE_CHARACTER_TYPE))",
        "    (CLASSVAR_STATIC_DECL",
        "      (VARIABLE_NAME string_value:var4)",
        "      (VARIABLE_CHARACTER_TYPE))))"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }

  SECTION("class and subroutine vars")
  {
    TextReader R(CLASSVARS_AND_SUBVARS_SRC);
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_class();
    std::string as_str = root.get().as_s_expression();
    Expected_t expected = {
        "(CLASS_DECL string_value:JackTest",
        "  (CLASSVAR_DECL_BLOCK",
        "    (CLASSVAR_STATIC_DECL",
        "      (VARIABLE_NAME string_value:inst1)",
        "      (VARIABLE_CLASS_TYPE string_value:ClassName))",
        "    (CLASSVAR_FIELD_DECL",
        "      (VARIABLE_NAME string_value:var1)",
        "      (VARIABLE_INTEGER_TYPE)))",
        "  (METHOD_DECL string_value:sub1",
        "    (SUBROUTINE_DESCR",
        "      (SUBROUTINE_DECL_RETURN_TYPE string_value:void)",
        "      (PARAMETER_LIST",
        "        (PARAMETER_DECL",
        "          (VARIABLE_NAME string_value:a1)",
        "          (VARIABLE_INTEGER_TYPE))",
        "        (PARAMETER_DECL",
        "          (VARIABLE_NAME string_value:a2)",
        "          (VARIABLE_INTEGER_TYPE))))",
        "    (SUBROUTINE_BODY",
        "      (LOCAL_VAR_DECL_BLOCK",
        "        (LOCAL_VAR_DECL",
        "          (VARIABLE_NAME string_value:v1)",
        "          (VARIABLE_INTEGER_TYPE))",
        "        (LOCAL_VAR_DECL",
        "          (VARIABLE_NAME string_value:v2)",
        "          (VARIABLE_BOOLEAN_TYPE))",
        "        (LOCAL_VAR_DECL",
        "          (VARIABLE_NAME string_value:v3)",
        "          (VARIABLE_BOOLEAN_TYPE)))",
        "      (STATEMENT_BLOCK",
        "        (RETURN_STATEMENT)))))"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }

  SECTION("constant void method call")
  {
    TextReader R(CONST_VOID_METHOD_CALL_SRC);
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_class();
    std::string as_str = root.get().as_s_expression();
    Expected_t expected = {
        "(CLASS_DECL string_value:Test",
        "  (METHOD_DECL string_value:draw",
        "    (SUBROUTINE_DESCR",
        "      (SUBROUTINE_DECL_RETURN_TYPE string_value:void))",
        "    (SUBROUTINE_BODY",
        "      (STATEMENT_BLOCK",
        "        (RETURN_STATEMENT))))",
        "  (METHOD_DECL string_value:run",
        "    (SUBROUTINE_DESCR",
        "      (SUBROUTINE_DECL_RETURN_TYPE string_value:void))",
        "    (SUBROUTINE_BODY",
        "      (STATEMENT_BLOCK",
        "        (DO_STATEMENT",
        "          (SUBROUTINE_CALL",
        "            (LOCAL_CALL_SITE",
        "              (SUBROUTINE_NAME string_value:draw))))",
        "        (RETURN_STATEMENT)))))"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

#if 0
  SECTION("object method call")
  {
    TextReader R(OBJECT_METHOD_CALL_SRC);
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_class();
    std::string as_str = root.get().as_s_expression();
    REQUIRE(as_str ==
        "(CLASS_DECL string_value:Main",
        "  (METHOD_DECL string_value:mul",
        "    (SUBROUTINE_DESCR",
        "      (SUBROUTINE_DECL_RETURN_TYPE string_value:int)",
        "      (PARAMETER_LIST",
        "        (PARAMETER_DECL",
        "          (VARIABLE_NAME string_value:a1)",
        "          (VARIABLE_INTEGER_TYPE))",
        "        (PARAMETER_DECL",
        "          (VARIABLE_NAME string_value:b)",
        "          (VARIABLE_INTEGER_TYPE))))",
        "        (RETURN_STATEMENT",
        "          (EXPRESSION",
        "            (TERM",
        "              (INTEGER_CONSTANT string_value:0)))))",



        "    (SUBROUTINE_BODY",
        "      (STATEMENT_BLOCK",
        "        (RETURN_STATEMENT))))"

        "  (METHOD_DECL string_value:run",
        "    (SUBROUTINE_DESCR",
        "      (SUBROUTINE_DECL_RETURN_TYPE string_value:void))",
        "    (SUBROUTINE_BODY",
        "      (STATEMENT_BLOCK",
        "        (SUBROUTINE_CALL",
        "          (CALL_SITE_BINDING",

        "            (BIND_NAME",
        "            (SUBROUTINE_NAME",

        "        (RETURN_STATEMENT))))"

        "    (SUBROUTINE_DESCR",
        "      (SUBROUTINE_DECL_RETURN_TYPE string_value:void)",
        "      (PARAMETER_LIST",
        "        (PARAMETER_DECL",
        "          (VARIABLE_NAME string_value:a1)",
        "          (VARIABLE_INTEGER_TYPE))",
        "        (PARAMETER_DECL",
        "          (VARIABLE_NAME string_value:a2)",
        "          (VARIABLE_INTEGER_TYPE))))",
        "    (SUBROUTINE_BODY",
        "      (LOCAL_VAR_DECL_BLOCK",
        "        (LOCAL_VAR_DECL",
        "          (VARIABLE_NAME string_value:v1)",
        "          (VARIABLE_INTEGER_TYPE))",
        "        (LOCAL_VAR_DECL",
        "          (VARIABLE_NAME string_value:v2)",
        "          (VARIABLE_BOOLEAN_TYPE))",
        "        (LOCAL_VAR_DECL",
        "          (VARIABLE_NAME string_value:v3)",
        "          (VARIABLE_BOOLEAN_TYPE)))",
        "      (STATEMENT_BLOCK",
        "        (RETURN_STATEMENT)))))"};








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
