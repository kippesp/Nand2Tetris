#include <iostream>

#include "common.h"
#include "jack_sources.h"

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
        "(INTEGER_CONSTANT integer_value:1)"};

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
        "(INTEGER_CONSTANT integer_value:1)"};

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
        "  (INTEGER_CONSTANT integer_value:1)",
        "  (INTEGER_CONSTANT integer_value:2))"};

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
        "  (INTEGER_CONSTANT integer_value:1)",
        "  (OP_DIVIDE",
        "    (INTEGER_CONSTANT integer_value:2)",
        "    (INTEGER_CONSTANT integer_value:3)))"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }

  SECTION("long multiply chain")
  {
    TextReader R("2 * 3 / 4 * 5 / 6 * 7");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_expression();
    std::string as_str = root.get().as_s_expression();

    Expected_t expected = {
        ""  // clang-format sorcery
        "(OP_MULTIPLY",
        "  (INTEGER_CONSTANT integer_value:2)",
        "  (OP_DIVIDE",
        "    (INTEGER_CONSTANT integer_value:3)",
        "    (OP_MULTIPLY",
        "      (INTEGER_CONSTANT integer_value:4)",
        "      (OP_DIVIDE",
        "        (INTEGER_CONSTANT integer_value:5)",
        "        (OP_MULTIPLY",
        "          (INTEGER_CONSTANT integer_value:6)",
        "          (INTEGER_CONSTANT integer_value:7))))))"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }

  SECTION("long expression chain, with precedence")
  {
    TextReader R("2 + 3 * 4 - 5 * 6 + 7");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_expression();
    std::string as_str = root.get().as_s_expression();

    Expected_t expected = {
        ""  // clang-format sorcery
        "(OP_ADD",
        "  (INTEGER_CONSTANT integer_value:2)",
        "  (OP_SUBTRACT",
        "    (OP_MULTIPLY",
        "      (INTEGER_CONSTANT integer_value:3)",
        "      (INTEGER_CONSTANT integer_value:4))",
        "    (OP_ADD",
        "      (OP_MULTIPLY",
        "        (INTEGER_CONSTANT integer_value:5)",
        "        (INTEGER_CONSTANT integer_value:6))",
        "      (INTEGER_CONSTANT integer_value:7))))"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }

  SECTION("long expression chain, left associative")
  {
    TextReader R("2 + 3 * 4 - 5 * 6 + 7");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    parser.set_left_associative();
    const auto& root = parser.parse_expression();
    std::string as_str = root.get().as_s_expression();

    Expected_t expected = {
        ""  // clang-format sorcery
        "(OP_ADD",
        "  (INTEGER_CONSTANT integer_value:2)",
        "  (OP_MULTIPLY",
        "    (INTEGER_CONSTANT integer_value:3)",
        "    (OP_SUBTRACT",
        "      (INTEGER_CONSTANT integer_value:4)",
        "      (OP_MULTIPLY",
        "        (INTEGER_CONSTANT integer_value:5)",
        "        (OP_ADD",
        "          (INTEGER_CONSTANT integer_value:6)",
        "          (INTEGER_CONSTANT integer_value:7))))))"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }

  SECTION("long paren expression chain, left associative w/parens")
  {
    TextReader R("2 + (3 * 4) - (5 * 6) + 7");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_expression();
    parser.set_left_associative();
    std::string as_str = root.get().as_s_expression();

    Expected_t expected = {
        ""  // clang-format sorcery
        "(OP_ADD",
        "  (INTEGER_CONSTANT integer_value:2)",
        "  (OP_SUBTRACT",
        "    (OP_MULTIPLY",
        "      (INTEGER_CONSTANT integer_value:3)",
        "      (INTEGER_CONSTANT integer_value:4))",
        "    (OP_ADD",
        "      (OP_MULTIPLY",
        "        (INTEGER_CONSTANT integer_value:5)",
        "        (INTEGER_CONSTANT integer_value:6))",
        "      (INTEGER_CONSTANT integer_value:7))))"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }

  SECTION("3-term add")
  {
    TextReader R("1 + 2 - 3");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_expression();
    std::string as_str = root.get().as_s_expression();

    Expected_t expected = {
        ""  // clang-format sorcery
        "(OP_ADD",
        "  (INTEGER_CONSTANT integer_value:1)",
        "  (OP_SUBTRACT",
        "    (INTEGER_CONSTANT integer_value:2)",
        "    (INTEGER_CONSTANT integer_value:3)))"};

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
        "  (INTEGER_CONSTANT integer_value:1)",
        "  (SUBROUTINE_CALL",
        "    (GLOBAL_CALL_SITE",
        "      (GLOBAL_BIND_NAME string_value:Math)",
        "      (SUBROUTINE_NAME string_value:square))",
        "    (CALL_ARGUMENTS",
        "      (INTEGER_CONSTANT integer_value:2))))"};

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
        "  (INTEGER_CONSTANT integer_value:2))"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }

  SECTION("prefix operator")
  {
    TextReader R("~(-1 < 0)");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_expression();
    std::string as_str = root.get().as_s_expression();

    Expected_t expected = {
        ""  // clang-format sorcery
        "(OP_PREFIX_LOGICAL_NOT",
        "  (OP_LOGICAL_LT",
        "    (OP_PREFIX_NEG",
        "      (INTEGER_CONSTANT integer_value:1))",
        "    (INTEGER_CONSTANT integer_value:0)))"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }

  SECTION("3-term statement in parens")
  {
    TextReader R("(1 + 2 + 3)");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_expression();
    std::string as_str = root.get().as_s_expression();

    Expected_t expected = {
        ""  // clang-format sorcery
        "(OP_ADD",
        "  (INTEGER_CONSTANT integer_value:1)",
        "  (OP_ADD",
        "    (INTEGER_CONSTANT integer_value:2)",
        "    (INTEGER_CONSTANT integer_value:3)))"};

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
        "    (SUBROUTINE_NAME string_value:fn))",
        "  (CALL_ARGUMENTS))"};

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
        "    (SUBROUTINE_NAME string_value:fn))",
        "  (CALL_ARGUMENTS))"};

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
        "  (CALL_ARGUMENTS",
        "    (INTEGER_CONSTANT integer_value:1)))"};

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
        "  (CALL_ARGUMENTS",
        "    (INTEGER_CONSTANT integer_value:1)",
        "    (INTEGER_CONSTANT integer_value:2)))"};

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
        "  (CALL_ARGUMENTS",
        "    (OP_MULTIPLY",
        "      (INTEGER_CONSTANT integer_value:2)",
        "      (INTEGER_CONSTANT integer_value:3))",
        "    (OP_MULTIPLY",
        "      (INTEGER_CONSTANT integer_value:4)",
        "      (INTEGER_CONSTANT integer_value:5))))"};

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
        "  (CALL_ARGUMENTS",
        "    (OP_MULTIPLY",
        "      (INTEGER_CONSTANT integer_value:2)",
        "      (OP_ADD",
        "        (INTEGER_CONSTANT integer_value:1)",
        "        (INTEGER_CONSTANT integer_value:3)))))"};

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
        "  (CALL_ARGUMENTS",
        "    (STRING_CONSTANT string_value:Hello)))"};

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
        "    (INTEGER_CONSTANT integer_value:1)",
        "    (SUBROUTINE_CALL",
        "      (GLOBAL_CALL_SITE",
        "        (GLOBAL_BIND_NAME string_value:Math)",
        "        (SUBROUTINE_NAME string_value:square))",
        "      (CALL_ARGUMENTS",
        "        (INTEGER_CONSTANT integer_value:2)))))"};

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
        "    (INTEGER_CONSTANT integer_value:0))",
        "  (SUBSCRIPTED_VARIABLE_NAME string_value:b",
        "    (OP_MULTIPLY",
        "      (INTEGER_CONSTANT integer_value:2)",
        "      (VARIABLE_NAME string_value:i))))"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }

  SECTION("let statement, 3-terms")
  {
    TextReader R("let base_address = 1 + 2 + 3;");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_statement();
    std::string as_str = root.get().as_s_expression();

    Expected_t expected = {
        ""  // clang-format sorcery
        "(LET_STATEMENT",
        "  (VARIABLE_NAME string_value:base_address)",
        "  (OP_ADD",
        "    (INTEGER_CONSTANT integer_value:1)",
        "    (OP_ADD",
        "      (INTEGER_CONSTANT integer_value:2)",
        "      (INTEGER_CONSTANT integer_value:3))))"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }

  SECTION("while statement")
  {
    TextReader R("while (a > 0) { let a = a - 1; }");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_statement();
    std::string as_str = root.get().as_s_expression();

    Expected_t expected = {
        ""
        "(WHILE_STATEMENT",
        "  (OP_LOGICAL_GT",
        "    (VARIABLE_NAME string_value:a)",
        "    (INTEGER_CONSTANT integer_value:0))",
        "  (STATEMENT_BLOCK",
        "    (LET_STATEMENT",
        "      (VARIABLE_NAME string_value:a)",
        "      (OP_SUBTRACT",
        "        (VARIABLE_NAME string_value:a)",
        "        (INTEGER_CONSTANT integer_value:1)))))"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }

  SECTION("while statement, empty body")
  {
    TextReader R("while (a > 0) { }");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_statement();
    std::string as_str = root.get().as_s_expression();

    Expected_t expected = {
        ""
        "(WHILE_STATEMENT",
        "  (OP_LOGICAL_GT",
        "    (VARIABLE_NAME string_value:a)",
        "    (INTEGER_CONSTANT integer_value:0))",
        "  (STATEMENT_BLOCK))"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }

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
        "  (OP_LOGICAL_EQUALS",
        "    (VARIABLE_NAME string_value:mask)",
        "    (INTEGER_CONSTANT integer_value:0))",
        "  (STATEMENT_BLOCK",
        "    (RETURN_STATEMENT",
        "      (INTEGER_CONSTANT integer_value:1)))",
        "  (STATEMENT_BLOCK",
        "    (RETURN_STATEMENT",
        "      (OP_MULTIPLY",
        "        (VARIABLE_NAME string_value:mask)",
        "        (INTEGER_CONSTANT integer_value:2)))))"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }

  SECTION("if statement, no else")
  {
    TextReader R("if (mask = 0) { return 1; }");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_statement();
    std::string as_str = root.get().as_s_expression();

    Expected_t expected = {
        ""  // clang-format sorcery
        "(IF_STATEMENT",
        "  (OP_LOGICAL_EQUALS",
        "    (VARIABLE_NAME string_value:mask)",
        "    (INTEGER_CONSTANT integer_value:0))",
        "  (STATEMENT_BLOCK",
        "    (RETURN_STATEMENT",
        "      (INTEGER_CONSTANT integer_value:1)))",
        "  (STATEMENT_BLOCK))"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }

  SECTION("do statement (local), no parms")
  {
    TextReader R("do local_run();");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_statement();
    std::string as_str = root.get().as_s_expression();

    Expected_t expected = {
        ""  // clang-format sorcery
        "(DO_STATEMENT",
        "  (SUBROUTINE_CALL",
        "    (LOCAL_CALL_SITE",
        "      (SUBROUTINE_NAME string_value:local_run))",
        "    (CALL_ARGUMENTS)))"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }

  SECTION("do statement, one arg")
  {
    TextReader R("do MyClass.run(true);");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_statement();
    std::string as_str = root.get().as_s_expression();

    Expected_t expected = {
        ""  // clang-format sorcery
        "(DO_STATEMENT",
        "  (SUBROUTINE_CALL",
        "    (GLOBAL_CALL_SITE",
        "      (GLOBAL_BIND_NAME string_value:MyClass)",
        "      (SUBROUTINE_NAME string_value:run))",
        "    (CALL_ARGUMENTS",
        "      (TRUE_KEYWORD))))"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }

  SECTION("do statement, multiple args")
  {
    TextReader R("do MyClass.run(1, 2 + (3 * 4));");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_statement();
    std::string as_str = root.get().as_s_expression();

    Expected_t expected = {
        ""  // clang-format sorcery
        "(DO_STATEMENT",
        "  (SUBROUTINE_CALL",
        "    (GLOBAL_CALL_SITE",
        "      (GLOBAL_BIND_NAME string_value:MyClass)",
        "      (SUBROUTINE_NAME string_value:run))",
        "    (CALL_ARGUMENTS",
        "      (INTEGER_CONSTANT integer_value:1)",
        "      (OP_ADD",
        "        (INTEGER_CONSTANT integer_value:2)",
        "        (OP_MULTIPLY",
        "          (INTEGER_CONSTANT integer_value:3)",
        "          (INTEGER_CONSTANT integer_value:4))))))",
    };

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }
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
        "          (INTEGER_CONSTANT integer_value:1))))))"};

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
        "          (THIS_KEYWORD)))))",
        "  (METHOD_DECL string_value:ref",
        "    (SUBROUTINE_DESCR",
        "      (RETURN_TYPE string_value:int))",
        "    (SUBROUTINE_BODY",
        "      (STATEMENT_BLOCK",
        "        (RETURN_STATEMENT",
        "          (THIS_KEYWORD))))))"};

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
        ""
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
        "              (SUBROUTINE_NAME string_value:draw))",
        "            (CALL_ARGUMENTS)))",
        "        (RETURN_STATEMENT)))))"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
  }
}
