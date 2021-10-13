#include <iostream>

#include "common.h"
#include "jack_sources.h"

#if 0
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

  SECTION("function call in expression")
  {
    TextReader R("1 + Math.square(2)");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_expression();
    std::string as_str = root.get().as_s_expression();

    Expected_t expected = {
        ""  // clang-format sorcery
        "(OP_ADD",
        "  (INTEGER_CONSTANT string_value:1)",
        "  (SUBROUTINE_CALL",
        "    (GLOBAL_CALL_SITE",
        "      (GLOBAL_BIND_NAME string_value:Math)",
        "      (SUBROUTINE_NAME string_value:square))",
        "    (INTEGER_CONSTANT string_value:2)))"};

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

  SECTION("scalar term")
  {
    TextReader R("b * 2");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_expression();
    std::string as_str = root.get().as_s_expression();

    Expected_t expected = {
        ""  // clang-format sorcery
        "(OP_MULTIPLY",
        "  (VARIABLE_NAME string_value:b)",
        "  (INTEGER_CONSTANT string_value:2))"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }

#ifdef LATER
  SECTION("prefix operator")
  {
    TextReader R("-1 - 2");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_expression();
    std::string as_str = root.get().as_s_expression();

    Expected_t expected = {
        ""  // clang-format sorcery
        "(OP_SUBTRACT",
        "  (PREFIX_OP_NEGATIVE
        "    (INTEGER_CONSTANT string_value:1))",
        "  (INTEGER_CONSTANT string_value:2))"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }
#endif
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

  SECTION("global call, expression parm")
  {
    TextReader R("Output.printInt(2 * (1 + 3))");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_subroutine_call();
    std::string as_str = root.get().as_s_expression();

    Expected_t expected = {
        ""  // clang-format sorcery
        "(SUBROUTINE_CALL",
        "  (GLOBAL_CALL_SITE",
        "    (GLOBAL_BIND_NAME string_value:Output)",
        "    (SUBROUTINE_NAME string_value:printInt))",
        "  (OP_MULTIPLY",
        "    (INTEGER_CONSTANT string_value:2)",
        "    (OP_ADD",
        "      (INTEGER_CONSTANT string_value:1)",
        "      (INTEGER_CONSTANT string_value:3))))"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }

  SECTION("global call, string parm")
  {
    TextReader R("Output.printString(\"Hello\")");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_subroutine_call();
    std::string as_str = root.get().as_s_expression();

    Expected_t expected = {
        ""  // clang-format sorcery
        "(SUBROUTINE_CALL",
        "  (GLOBAL_CALL_SITE",
        "    (GLOBAL_BIND_NAME string_value:Output)",
        "    (SUBROUTINE_NAME string_value:printString))",
        "  (STRING_CONSTANT string_value:Hello))"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }
}

SCENARIO("Parse statements")
{
  SECTION("let statement")
  {
    TextReader R("let i = 1 + Math.square(2);");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_statement();
    std::string as_str = root.get().as_s_expression();

    Expected_t expected = {
        ""  // clang-format sorcery
        "(LET_STATEMENT",
        "  (VARIABLE_NAME string_value:i)",
        "  (OP_ADD",
        "    (INTEGER_CONSTANT string_value:1)",
        "    (SUBROUTINE_CALL",
        "      (GLOBAL_CALL_SITE",
        "        (GLOBAL_BIND_NAME string_value:Math)",
        "        (SUBROUTINE_NAME string_value:square))",
        "      (INTEGER_CONSTANT string_value:2))))"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }

  SECTION("let statement, arrays")
  {
    TextReader R("let a[0] = b[2 * i];");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_statement();
    std::string as_str = root.get().as_s_expression();

    Expected_t expected = {
        ""  // clang-format sorcery
        "(LET_STATEMENT",
        "  (SUBSCRIPTED_VARIABLE_NAME string_value:a",
        "    (INTEGER_CONSTANT string_value:0))",
        "  (SUBSCRIPTED_VARIABLE_NAME string_value:b",
        "    (OP_MULTIPLY",
        "      (INTEGER_CONSTANT string_value:2)",
        "      (VARIABLE_NAME string_value:i))))"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }

#ifdef LATER
  SECTION("if statement")
  {
    TextReader R("if (mask = 0) { return 1; } else { return mask * 2; }");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_statement();
    std::string as_str = root.get().as_s_expression();

    Expected_t expected = {
        ""  // clang-format sorcery
        "(IF_STATEMENT",
        "      (INTEGER_CONSTANT string_value:1))))"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }
#endif
}

SCENARIO("Parse tree basics")
{
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
        "      (RETURN_TYPE string_value:int))",
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
        "      (RETURN_TYPE string_value:void))",
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
        "      (RETURN_TYPE string_value:Test))",
        "    (SUBROUTINE_BODY",
        "      (STATEMENT_BLOCK",
        "        (RETURN_STATEMENT",
        "          (KEYWORD_CONSTANT",
        "            (THIS_KEYWORD))))))",
        "  (METHOD_DECL string_value:ref",
        "    (SUBROUTINE_DESCR",
        "      (RETURN_TYPE string_value:int))",
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
        "  (CLASS_VARIABLES",
        "    (VARIABLE_DECL string_value:inst1",
        "      (CLASS_VARIABLE_SCOPE string_value:static)",
        "      (VARIABLE_TYPE string_value:ClassName))",
        "    (VARIABLE_DECL string_value:var1",
        "      (CLASS_VARIABLE_SCOPE string_value:field)",
        "      (VARIABLE_TYPE string_value:int))",
        "    (VARIABLE_DECL string_value:var2",
        "      (CLASS_VARIABLE_SCOPE string_value:field)",
        "      (VARIABLE_TYPE string_value:boolean))",
        "    (VARIABLE_DECL string_value:var3",
        "      (CLASS_VARIABLE_SCOPE string_value:static)",
        "      (VARIABLE_TYPE string_value:char))",
        "    (VARIABLE_DECL string_value:var4",
        "      (CLASS_VARIABLE_SCOPE string_value:static)",
        "      (VARIABLE_TYPE string_value:char))))"};

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
        ""
        "(CLASS_DECL string_value:JackTest",
        "  (CLASS_VARIABLES",
        "    (VARIABLE_DECL string_value:inst1",
        "      (CLASS_VARIABLE_SCOPE string_value:static)",
        "      (VARIABLE_TYPE string_value:ClassName))",
        "    (VARIABLE_DECL string_value:var1",
        "      (CLASS_VARIABLE_SCOPE string_value:field)",
        "      (VARIABLE_TYPE string_value:int)))",
        "  (METHOD_DECL string_value:sub1",
        "    (SUBROUTINE_DESCR",
        "      (RETURN_TYPE string_value:void)",
        "      (INPUT_PARAMETERS",
        "        (VARIABLE_DECL string_value:a1",
        "          (VARIABLE_TYPE string_value:int))",
        "        (VARIABLE_DECL string_value:a2",
        "          (VARIABLE_TYPE string_value:int)))",
        "      (LOCAL_VARIABLES",
        "        (VARIABLE_DECL string_value:v1",
        "          (VARIABLE_TYPE string_value:int))",
        "        (VARIABLE_DECL string_value:v2",
        "          (VARIABLE_TYPE string_value:boolean))",
        "        (VARIABLE_DECL string_value:v3",
        "          (VARIABLE_TYPE string_value:boolean))))",
        "    (SUBROUTINE_BODY",
        "      (STATEMENT_BLOCK",
        "        (RETURN_STATEMENT)))))"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }

  SECTION("local method call, no parms")
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
        "      (RETURN_TYPE string_value:void))",
        "    (SUBROUTINE_BODY",
        "      (STATEMENT_BLOCK",
        "        (RETURN_STATEMENT))))",
        "  (METHOD_DECL string_value:run",
        "    (SUBROUTINE_DESCR",
        "      (RETURN_TYPE string_value:void))",
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

#ifdef LATER
  SECTION("local method call, two parms")
  {
    TextReader R(CLASS_METHOD_CALL_SRC);
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_class();
    std::string as_str = root.get().as_s_expression();
    Expected_t expected = {
        "(CLASS_DECL string_value:Main",
        "  (METHOD_DECL string_value:mul",
        "    (SUBROUTINE_DESCR",
        "      (RETURN_TYPE string_value:int)",
        "      (VARIABLE_DECL",
        "        (VARIABLE_NAME string_value:a)",
        "        (VARIABLE_TYPE string_value:int))",
        "      (VARIABLE_DECL",
        "        (VARIABLE_NAME string_value:b)",
        "        (VARIABLE_TYPE string_value:int)))",
        "    (SUBROUTINE_BODY",
        "      (STATEMENT_BLOCK",
        "        (RETURN_STATEMENT",
        "          (INTEGER_CONSTANT string_value:0)))))",
        "  (METHOD_DECL string_value:f1",
        "    (SUBROUTINE_DESCR",
        "      (RETURN_TYPE string_value:int))",
        "    (SUBROUTINE_BODY",
        "      (STATEMENT_BLOCK",
        "        (RETURN_STATEMENT",
        "          (SUBROUTINE_CALL",
        "            (LOCAL_CALL_SITE",
        "              (SUBROUTINE_NAME string_value:mul))",
        "            (INTEGER_CONSTANT string_value:1)"
        "            (INTEGER_CONSTANT string_value:2)))))))"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }
#endif

#if 0
  SECTION("local method call, no parms")
  {
    TextReader R(CLASS_METHOD_CALL_SRC);
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_class();
    std::string as_str = root.get().as_s_expression();
    Expected_t expected = {
        "(CLASS_DECL string_value:Test",
        "  (METHOD_DECL string_value:draw",
        "    (SUBROUTINE_DESCR",
        "      (RETURN_TYPE string_value:void))",
        "    (SUBROUTINE_BODY",
        "      (STATEMENT_BLOCK",
        "        (RETURN_STATEMENT))))",
        "  (METHOD_DECL string_value:run",
        "    (SUBROUTINE_DESCR",
        "      (RETURN_TYPE string_value:void))",
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
#endif
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

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
            "      (RETURN_TYPE string_value:int))\n"
            "    (SUBROUTINE_BODY\n"
            "      (STATEMENT_BLOCK\n"
            "        (RETURN_STATEMENT\n"
            "          (EXPRESSION\n"
            "            (TERM\n"
            "              (INTEGER_CONSTANT string_value:1))))))))");
  }
#endif
