#include "common.h"

#include <iostream>

SCENARIO("Parse tree simple terms")
{
  SECTION("simple let")
  {
    TextReader R("let i = true;");

    JackTokenizer T(R);
    JackTokenizer::Tokens_t tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_let_statement();

    // (N_LET_STATEMENT
    //   (N_SCALAR_VAR
    //     (N_VARIABLE_NAME i))
    //   (N_EXPRESSION
    //     (N_TERM
    //       (N_KEYWORD_CONSTANT
    //         (N_TRUE_KEYWORD)))))

    // Manually check tree structure

    REQUIRE(root.get().type == ast::AstNodeType_t::N_LET_STATEMENT);

    const auto root_1 = root.get().get_child_nodes();
    REQUIRE(root_1[0].get().type == ast::AstNodeType_t::N_SCALAR_VAR);
    REQUIRE(root_1[1].get().type == ast::AstNodeType_t::N_EXPRESSION);

    const auto root_1_1 = root_1[0].get().get_child_nodes();
    REQUIRE(root_1_1[0].get().type == ast::AstNodeType_t::N_VARIABLE_NAME);
    REQUIRE(std::get<std::string>(root_1_1[0].get().value) == "i");

    const auto root_2_1 = root_1[1].get().get_child_nodes();
    REQUIRE(root_2_1[0].get().type == ast::AstNodeType_t::N_TERM);

    const auto root_2_1_1 = root_2_1[0].get().get_child_nodes();
    REQUIRE(root_2_1_1[0].get().type == ast::AstNodeType_t::N_KEYWORD_CONSTANT);

    const auto root_2_1_1_1 = root_2_1_1[0].get().get_child_nodes();
    REQUIRE(root_2_1_1_1[0].get().type == ast::AstNodeType_t::N_TRUE_KEYWORD);

    // Compare using string representation

    std::string as_str = root.get().as_s_expression();
    REQUIRE(as_str ==
            "(LET_STATEMENT\n"
            "  (SCALAR_VAR\n"
            "    (VARIABLE_NAME string_value:i))\n"
            "  (EXPRESSION\n"
            "    (TERM\n"
            "      (KEYWORD_CONSTANT\n"
            "        (TRUE_KEYWORD)))))");
  }

  SECTION("simple seven class")
  {
    TextReader R(
        "class ConstSeven\n"
        "{\n"
        "  function int get_seven()\n"
        "  {\n"
        "    return 7;\n"
        "  }\n"
        "}\n");

    JackTokenizer T(R);
    JackTokenizer::Tokens_t tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);
    const auto& root = parser.parse_class();
    std::string as_str = root.get().as_s_expression();
    REQUIRE(as_str ==
            "(CLASS_DECL string_value:ConstSeven\n"
            "  (FUNCTION_DECL string_value:get_seven\n"
            "    (SUBROUTINE_DESCR\n"
            "      (SUBROUTINE_DECL_RETURN_TYPE string_value:int))\n"
            "    (SUBROUTINE_BODY\n"
            "      (STATEMENT_BLOCK\n"
            "        (RETURN_STATEMENT\n"
            "          (EXPRESSION\n"
            "            (TERM\n"
            "              (INTEGER_CONSTANT string_value:7))))))))");
  }
}
