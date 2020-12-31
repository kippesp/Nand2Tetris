#include <string.h>

#include "catch.hpp"
#include "mock_reader.h"
#include "parser/parse_tree.h"

using namespace std;

SCENARIO("Parse tree simple terms")
{
  test::MockReader R;

  SECTION("simple integer term")
  {
    //         <term>
    //           |
    //  <integer-constant>:1
    strcpy(R.buffer, "1");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto root = T.get_root();
    REQUIRE(root->type == ParseTreeNodeType_t::P_UNDEFINED);

    // verify base of the term is a non-terminal and a P_TERM
    auto parsetree_node = T.parse_term();
    auto nonterm_node =
        dynamic_pointer_cast<ParseTreeNonTerminal>(parsetree_node);
    REQUIRE(nonterm_node);
    REQUIRE(nonterm_node->type == ParseTreeNodeType_t::P_TERM);
    auto failed_cast = dynamic_pointer_cast<ParseTreeTerminal>(parsetree_node);
    REQUIRE(failed_cast == nullptr);

    REQUIRE(nonterm_node->num_child_nodes() == 1);

    auto child_nodes = nonterm_node->get_child_nodes();

    auto term_node = dynamic_pointer_cast<ParseTreeTerminal>(child_nodes[0]);
    REQUIRE(term_node);
    REQUIRE(term_node->type == ParseTreeNodeType_t::P_INTEGER_CONSTANT);
    REQUIRE(term_node->token.value_str == "1");

    // verify tree structure using strings
    stringstream ss;
    ss << *term_node;
    REQUIRE(ss.str() == "(P_INTEGER_CONSTANT 1)");

    ss.str("");
    ss << *nonterm_node;
    REQUIRE(ss.str() == "(P_TERM (P_INTEGER_CONSTANT 1))");
  }

  SECTION("non-term")
  {
    // <class>
    strcpy(R.buffer, "class Class {}");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_term();
    auto nonterm_node =
        dynamic_pointer_cast<ParseTreeNonTerminal>(parsetree_node);
    REQUIRE(!nonterm_node);

    auto term_node = dynamic_pointer_cast<ParseTreeTerminal>(parsetree_node);
    REQUIRE(!term_node);
  }

#if 0
  // a null expression isn't allowed
  SECTION("non-term w/null expression")
  {
    strcpy(R.buffer, "()");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_term();
    auto nonterm_node =
        dynamic_pointer_cast<ParseTreeNonTerminal>(parsetree_node);
    REQUIRE(!nonterm_node);

    auto term_node = dynamic_pointer_cast<ParseTreeTerminal>(parsetree_node);
    REQUIRE(!term_node);
  }
#endif

#if 0
  // a null array expression isn't allowed
  SECTION("non-term w/null array expression")
  {
    strcpy(R.buffer, "myvar[]");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_term();
    auto nonterm_node =
        dynamic_pointer_cast<ParseTreeNonTerminal>(parsetree_node);
    REQUIRE(!nonterm_node);

    auto term_node = dynamic_pointer_cast<ParseTreeTerminal>(parsetree_node);
    REQUIRE(!term_node);
  }
#endif

  SECTION("simple string term")
  {
    //         <term>
    //           |
    //  <string-constant>:string
    strcpy(R.buffer, "\"string\"");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);

    auto parsetree_node = T.parse_term();
    auto nonterm_node =
        dynamic_pointer_cast<ParseTreeNonTerminal>(parsetree_node);
    REQUIRE(nonterm_node);

    stringstream ss;
    ss << *nonterm_node;
    REQUIRE(ss.str() == "(P_TERM (P_STRING_CONSTANT string))");
  }

  SECTION("simple keyword term")
  {
    //         <term>
    //           |
    //  <keyword-constant>:true
    strcpy(R.buffer, "true");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);

    auto parsetree_node = T.parse_term();
    REQUIRE(parsetree_node);

    stringstream ss;
    ss << *parsetree_node;
    REQUIRE(ss.str() == "(P_TERM (P_KEYWORD_CONSTANT true))");
  }

  SECTION("scalar variable term")
  {
    //        <term>
    //          |
    //  <scalar-var>:myvar
    strcpy(R.buffer, "myvar");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);

    auto parsetree_node = T.parse_term();
    REQUIRE(parsetree_node);

    stringstream ss;
    ss << *parsetree_node;
    REQUIRE(ss.str() == "(P_TERM (P_SCALAR_VAR myvar))");
  }

  SECTION("array variable term")
  {
    //        <term>
    //          | \
    //          |  \--------+------------+----------+
    //          |           |            |          |
    //  <array-var>:myvar  "["      <expression>   "]"
    //                                   |
    //                                <term>
    //                                   |
    //                          <integer-constant>:1
    strcpy(R.buffer, "myvar[1]");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);

    auto parsetree_node = T.parse_term();
    REQUIRE(parsetree_node);

    stringstream ss;
    ss << *parsetree_node;
    REQUIRE(ss.str() ==
            "(P_TERM (P_ARRAY_VAR myvar)(P_LEFT_BRACKET <left_bracket>)"
            "(P_EXPRESSION (P_TERM (P_INTEGER_CONSTANT 1)))"
            "(P_RIGHT_BRACKET <right_bracket>))");
  }

  SECTION("grouped expresson term")
  {
    // (P_EXPRESSION
    //   (P_TERM
    //     (P_LEFT_PARENTHESIS left_parenthesis( ( ))
    //     (P_EXPRESSION
    //       (P_TERM
    //         (P_INTEGER_CONSTANT 1))
    //       (P_OP plus( + ))
    //       (P_TERM
    //         (P_INTEGER_CONSTANT 1)))
    //     (P_RIGHT_PARENTHESIS right_parenthesis( ) ))))
    strcpy(R.buffer, "(1 + 1)");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_expression();
    REQUIRE(parsetree_node);

    // verify tree structure using strings
    stringstream ss;
    ss << *parsetree_node;
    REQUIRE(ss.str() ==
            "(P_EXPRESSION "
            "(P_TERM (P_LEFT_PARENTHESIS <left_parenthesis>)"
            "(P_EXPRESSION (P_TERM (P_INTEGER_CONSTANT 1))"
            "(P_OP <plus>)(P_TERM (P_INTEGER_CONSTANT 1)))"
            "(P_RIGHT_PARENTHESIS <right_parenthesis>)))");
  }

  SECTION("grouped unary expresson term")
  {
    // (P_EXPRESSION
    //   (P_TERM
    //     (P_LEFT_PARENTHESIS left_parenthesis)
    //     (P_EXPRESSION
    //       (P_TERM
    //         (P_INTEGER_CONSTANT 1)))
    //     (P_RIGHT_PARENTHESIS right_parenthesis)))
    strcpy(R.buffer, "(1)");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_expression();
    REQUIRE(parsetree_node);

    // verify tree structure using strings
    stringstream ss;
    ss << *parsetree_node;
    REQUIRE(ss.str() ==
            "(P_EXPRESSION "
            "(P_TERM (P_LEFT_PARENTHESIS <left_parenthesis>)"
            "(P_EXPRESSION (P_TERM (P_INTEGER_CONSTANT 1)))"
            "(P_RIGHT_PARENTHESIS <right_parenthesis>)))");
  }

  SECTION("unary op - term")
  {
    //          <term>
    //            |
    //  <<unary-op>:- <term>>
    //                   |
    //          <integer-constant>:1
    strcpy(R.buffer, "- 1");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);

    auto parsetree_node = T.parse_term();
    REQUIRE(parsetree_node);

    stringstream ss;
    ss << *parsetree_node;
    REQUIRE(ss.str() ==
            "(P_TERM (P_UNARY_OP <minus>)(P_TERM (P_INTEGER_CONSTANT 1)))");
  }

  SECTION("unary op ~ term")
  {
    //          <term>
    //            |
    //  <<unary-op>:~ <term>>
    //                   |
    //          <integer-constant>:1
    strcpy(R.buffer, "~1");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);

    auto parsetree_node = T.parse_term();
    REQUIRE(parsetree_node);

    stringstream ss;
    ss << *parsetree_node;
    REQUIRE(ss.str() ==
            "(P_TERM (P_UNARY_OP <tilde>)(P_TERM (P_INTEGER_CONSTANT 1)))");
  }
}

SCENARIO("Parse tree expressions")
{
  test::MockReader R;

  SECTION("single term expresson")
  {
    //       <expression>
    //            |
    //         <term>
    //            |
    //   <integer-constant>:1
    //
    //  (P_EXPRESSION
    //    (P_TERM (P_INTEGER_CONSTANT 1)))
    strcpy(R.buffer, "1");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_expression();
    REQUIRE(parsetree_node);

    // verify tree structure using strings
    stringstream ss;
    ss << *parsetree_node;
    REQUIRE(ss.str() == "(P_EXPRESSION (P_TERM (P_INTEGER_CONSTANT 1)))");
  }

  SECTION("single-op keyword expresson")
  {
    //  (P_EXPRESSION
    //    (P_TERM (P_KEYWORD_CONSTANT true))
    //    (P_OP +)
    //    (P_TERM (P_KEYWORD_CONSTANT false))
    strcpy(R.buffer, "true+false");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_expression();
    REQUIRE(parsetree_node);

    // verify tree structure using strings
    stringstream ss;
    ss << *parsetree_node;
    REQUIRE(ss.str() ==
            "(P_EXPRESSION "
            "(P_TERM (P_KEYWORD_CONSTANT true))(P_OP <plus>)"
            "(P_TERM (P_KEYWORD_CONSTANT false)))");
  }

  SECTION("multi-op keyword expresson")
  {
    //  (P_EXPRESSION
    //    (P_TERM (P_KEYWORD_CONSTANT true))
    //    (P_OP +)
    //    (P_TERM (P_KEYWORD_CONSTANT false))
    //    (P_OP +)
    //    (P_TERM (P_KEYWORD_CONSTANT null))
    //    (P_OP +)
    //    (P_TERM (P_KEYWORD_CONSTANT this)))
    strcpy(R.buffer, "true + false+null + this");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_expression();
    REQUIRE(parsetree_node);

    // verify tree structure using strings
    stringstream ss;
    ss << *parsetree_node;
    REQUIRE(ss.str() ==
            "(P_EXPRESSION "
            "(P_TERM (P_KEYWORD_CONSTANT true))(P_OP <plus>)"
            "(P_TERM (P_KEYWORD_CONSTANT false))(P_OP <plus>)"
            "(P_TERM (P_KEYWORD_CONSTANT null))(P_OP <plus>)"
            "(P_TERM (P_KEYWORD_CONSTANT this)))");
  }

  SECTION("math ops expresson")
  {
    strcpy(R.buffer, "1 + 2 - 3 * 5 / 6 & 7 | 8");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_expression();
    REQUIRE(parsetree_node);

    // verify tree structure using strings
    stringstream ss;
    ss << *parsetree_node;
    REQUIRE(ss.str() ==
            "(P_EXPRESSION "
            "(P_TERM (P_INTEGER_CONSTANT 1))"
            "(P_OP <plus>)"
            "(P_TERM (P_INTEGER_CONSTANT 2))"
            "(P_OP <minus>)"
            "(P_TERM (P_INTEGER_CONSTANT 3))"
            "(P_OP <asterisk>)"
            "(P_TERM (P_INTEGER_CONSTANT 5))"
            "(P_OP <divide>)"
            "(P_TERM (P_INTEGER_CONSTANT 6))"
            "(P_OP <ampersand>)"
            "(P_TERM (P_INTEGER_CONSTANT 7))"
            "(P_OP <vbar>)"
            "(P_TERM (P_INTEGER_CONSTANT 8)))");
  }

  SECTION("logical ops expresson")
  {
    strcpy(R.buffer, "1 < 2 > 3 = 4");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_expression();
    REQUIRE(parsetree_node);

    // verify tree structure using strings
    stringstream ss;
    ss << *parsetree_node;
    REQUIRE(ss.str() ==
            "(P_EXPRESSION "
            "(P_TERM (P_INTEGER_CONSTANT 1))"
            "(P_OP <less_than>)"
            "(P_TERM (P_INTEGER_CONSTANT 2))"
            "(P_OP <greater_than>)"
            "(P_TERM (P_INTEGER_CONSTANT 3))"
            "(P_OP <equal>)"
            "(P_TERM (P_INTEGER_CONSTANT 4)))");
  }

  SECTION("long unary expresson")
  {
    strcpy(R.buffer, "1 + -2 + ~3");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_expression();
    REQUIRE(parsetree_node);

    // verify tree structure using strings
    stringstream ss;
    ss << *parsetree_node;
    REQUIRE(ss.str() ==
            // clang-format off
            "(P_EXPRESSION "
            "(P_TERM (P_INTEGER_CONSTANT 1))"
            "(P_OP <plus>)"
            "(P_TERM "
              "(P_UNARY_OP <minus>)"
              "(P_TERM (P_INTEGER_CONSTANT 2)))"
            "(P_OP <plus>)"
            "(P_TERM "
              "(P_UNARY_OP <tilde>)"
              "(P_TERM (P_INTEGER_CONSTANT 3))))"
            // clang-format on
    );
  }
}

SCENARIO("Parse tree subroutine terms")
{
  test::MockReader R;

  SECTION("class method call w/no expression")
  {
    // <subroutine-call>  ::= <subroutine-name> "(" ")"
    strcpy(R.buffer, "mymethod()");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_expression();
    REQUIRE(parsetree_node);

    // verify tree structure using strings
    stringstream ss;
    ss << *parsetree_node;
    REQUIRE(ss.str() ==
            "(P_EXPRESSION (P_TERM "
            "(P_SUBROUTINE_CALL "
            "(P_SUBROUTINE_NAME mymethod)"
            "(P_LEFT_PARENTHESIS <left_parenthesis>)"
            "(P_EXPRESSION_LIST )"
            "(P_RIGHT_PARENTHESIS <right_parenthesis>))))");
  }

  SECTION("class method call w/one expression")
  {
    // <subroutine-call> ::= <subroutine-name> "(" <expression> ")"
    strcpy(R.buffer, "mymethod(1)");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_expression();
    REQUIRE(parsetree_node);

    // verify tree structure using strings
    stringstream ss;
    ss << *parsetree_node;
    REQUIRE(ss.str() ==
            "(P_EXPRESSION (P_TERM "
            "(P_SUBROUTINE_CALL "
            "(P_SUBROUTINE_NAME mymethod)"
            "(P_LEFT_PARENTHESIS <left_parenthesis>)"
            "(P_EXPRESSION_LIST "
            "(P_EXPRESSION (P_TERM (P_INTEGER_CONSTANT 1))))"
            "(P_RIGHT_PARENTHESIS <right_parenthesis>))))");
  }

  SECTION("class method call w/expression list")
  {
    // <subroutine-call> ::= <subroutine-name> "(" <expression>,<expression> ")"
    strcpy(R.buffer, "mymethod(1, 2)");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_expression();
    REQUIRE(parsetree_node);

    // verify tree structure using strings
    stringstream ss;
    ss << *parsetree_node;
    REQUIRE(ss.str() ==
            "(P_EXPRESSION (P_TERM "
            "(P_SUBROUTINE_CALL "
            "(P_SUBROUTINE_NAME mymethod)"
            "(P_LEFT_PARENTHESIS <left_parenthesis>)"
            "(P_EXPRESSION_LIST "
            "(P_EXPRESSION (P_TERM (P_INTEGER_CONSTANT 1)))"
            "(P_DELIMITER <comma>)"
            "(P_EXPRESSION (P_TERM (P_INTEGER_CONSTANT 2))))"
            "(P_RIGHT_PARENTHESIS <right_parenthesis>))))");
  }

  SECTION("dotted class method call w/no expression")
  {
    // <subroutine-call>  ::= (<class-name> | <var-name>) "."
    //                        <subroutine-name> "(" ")"
    strcpy(R.buffer, "identifier.mymethod()");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_expression();
    REQUIRE(parsetree_node);

    // verify tree structure using strings
    stringstream ss;
    ss << *parsetree_node;
    REQUIRE(ss.str() ==
            "(P_EXPRESSION (P_TERM "
            "(P_SUBROUTINE_CALL "
            "(P_CLASS_OR_VAR_NAME identifier)"
            "(P_DELIMITER <period>)"
            "(P_SUBROUTINE_NAME mymethod)"
            "(P_LEFT_PARENTHESIS <left_parenthesis>)"
            "(P_EXPRESSION_LIST )"
            "(P_RIGHT_PARENTHESIS <right_parenthesis>))))");
  }

  SECTION("dotted class method call w/one expression")
  {
    // <subroutine-call> ::= (<class-name> | <var-name>) "."
    //                       <subroutine-name> "(" <expression> ")"
    strcpy(R.buffer, "identifier.mymethod(1)");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_expression();
    REQUIRE(parsetree_node);

    // verify tree structure using strings
    stringstream ss;
    ss << *parsetree_node;
    REQUIRE(ss.str() ==
            "(P_EXPRESSION (P_TERM "
            "(P_SUBROUTINE_CALL "
            "(P_CLASS_OR_VAR_NAME identifier)"
            "(P_DELIMITER <period>)"
            "(P_SUBROUTINE_NAME mymethod)"
            "(P_LEFT_PARENTHESIS <left_parenthesis>)"
            "(P_EXPRESSION_LIST "
            "(P_EXPRESSION (P_TERM (P_INTEGER_CONSTANT 1))))"
            "(P_RIGHT_PARENTHESIS <right_parenthesis>))))");
  }

  SECTION("dotted class method call w/expression list")
  {
    // <subroutine-call> ::= (<class-name> | <var-name>) "."
    //                       <subroutine-name> "(" <expression>,<expression> ")"
    strcpy(R.buffer, "identifier.mymethod(1, 2)");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_expression();
    REQUIRE(parsetree_node);

    // verify tree structure using strings
    stringstream ss;
    ss << *parsetree_node;
    REQUIRE(ss.str() ==
            "(P_EXPRESSION (P_TERM "
            "(P_SUBROUTINE_CALL "
            "(P_CLASS_OR_VAR_NAME identifier)"
            "(P_DELIMITER <period>)"
            "(P_SUBROUTINE_NAME mymethod)"
            "(P_LEFT_PARENTHESIS <left_parenthesis>)"
            "(P_EXPRESSION_LIST "
            "(P_EXPRESSION (P_TERM (P_INTEGER_CONSTANT 1)))"
            "(P_DELIMITER <comma>)"
            "(P_EXPRESSION (P_TERM (P_INTEGER_CONSTANT 2))))"
            "(P_RIGHT_PARENTHESIS <right_parenthesis>))))");
  }
}

SCENARIO("Parse tree statements")
{
  test::MockReader R;

  SECTION("return empty expression statement")
  {
    // <return-statement> ::= "return" {<expression>}? ";"
    strcpy(R.buffer, "return;");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_statements();
    REQUIRE(parsetree_node);

    // verify tree structure using strings
    stringstream ss;
    ss << *parsetree_node;
    REQUIRE(ss.str() ==
            "(P_STATEMENT_LIST "
            "(P_RETURN_STATEMENT "
            "(P_KEYWORD return)"
            "(P_DELIMITER <semicolon>)))");
  }

  SECTION("return with expression statement")
  {
    // <return-statement> ::= "return" {<expression>}? ";"
    strcpy(R.buffer, "return 1 = 2;");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_statements();
    REQUIRE(parsetree_node);

    // verify tree structure using strings
    stringstream ss;
    ss << *parsetree_node;
    REQUIRE(ss.str() ==
            "(P_STATEMENT_LIST "
            "(P_RETURN_STATEMENT "
            "(P_KEYWORD return)"
            "(P_EXPRESSION (P_TERM (P_INTEGER_CONSTANT 1))"
            "(P_OP <equal>)(P_TERM (P_INTEGER_CONSTANT 2)))"
            "(P_DELIMITER <semicolon>)))");
  }
}
