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
            "(P_TERM (P_ARRAY_VAR myvar)(P_DELIMITER <left_bracket>)"
            "(P_EXPRESSION (P_TERM (P_INTEGER_CONSTANT 1)))"
            "(P_DELIMITER <right_bracket>))");
  }

  SECTION("grouped expresson term")
  {
    // (P_EXPRESSION
    //   (P_TERM
    //     (P_DELIMITER left_parenthesis( ( ))
    //     (P_EXPRESSION
    //       (P_TERM
    //         (P_INTEGER_CONSTANT 1))
    //       (P_OP plus( + ))
    //       (P_TERM
    //         (P_INTEGER_CONSTANT 1)))
    //     (P_DELIMITER right_parenthesis( ) ))))
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
            "(P_TERM (P_DELIMITER <left_parenthesis>)"
            "(P_EXPRESSION (P_TERM (P_INTEGER_CONSTANT 1))"
            "(P_OP <plus>)(P_TERM (P_INTEGER_CONSTANT 1)))"
            "(P_DELIMITER <right_parenthesis>)))");
  }

  SECTION("grouped unary expresson term")
  {
    // (P_EXPRESSION
    //   (P_TERM
    //     (P_DELIMITER left_parenthesis)
    //     (P_EXPRESSION
    //       (P_TERM
    //         (P_INTEGER_CONSTANT 1)))
    //     (P_DELIMITER right_parenthesis)))
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
            "(P_TERM (P_DELIMITER <left_parenthesis>)"
            "(P_EXPRESSION (P_TERM (P_INTEGER_CONSTANT 1)))"
            "(P_DELIMITER <right_parenthesis>)))");
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
            "(P_SUBROUTINE_CALL_SITE_BINDING "
            "(P_SUBROUTINE_NAME mymethod))"
            "(P_DELIMITER <left_parenthesis>)"
            "(P_EXPRESSION_LIST )"
            "(P_DELIMITER <right_parenthesis>))))");
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
            "(P_SUBROUTINE_CALL_SITE_BINDING "
            "(P_SUBROUTINE_NAME mymethod))"
            "(P_DELIMITER <left_parenthesis>)"
            "(P_EXPRESSION_LIST "
            "(P_EXPRESSION (P_TERM (P_INTEGER_CONSTANT 1))))"
            "(P_DELIMITER <right_parenthesis>))))");
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
            "(P_SUBROUTINE_CALL_SITE_BINDING "
            "(P_SUBROUTINE_NAME mymethod))"
            "(P_DELIMITER <left_parenthesis>)"
            "(P_EXPRESSION_LIST "
            "(P_EXPRESSION (P_TERM (P_INTEGER_CONSTANT 1)))"
            "(P_DELIMITER <comma>)"
            "(P_EXPRESSION (P_TERM (P_INTEGER_CONSTANT 2))))"
            "(P_DELIMITER <right_parenthesis>))))");
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
            "(P_SUBROUTINE_CALL_SITE_BINDING "
            "(P_CLASS_OR_VAR_NAME identifier)"
            "(P_DELIMITER <period>)"
            "(P_SUBROUTINE_NAME mymethod))"
            "(P_DELIMITER <left_parenthesis>)"
            "(P_EXPRESSION_LIST )"
            "(P_DELIMITER <right_parenthesis>))))");
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
            "(P_SUBROUTINE_CALL_SITE_BINDING "
            "(P_CLASS_OR_VAR_NAME identifier)"
            "(P_DELIMITER <period>)"
            "(P_SUBROUTINE_NAME mymethod))"
            "(P_DELIMITER <left_parenthesis>)"
            "(P_EXPRESSION_LIST "
            "(P_EXPRESSION (P_TERM (P_INTEGER_CONSTANT 1))))"
            "(P_DELIMITER <right_parenthesis>))))");
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
            "(P_SUBROUTINE_CALL_SITE_BINDING "
            "(P_CLASS_OR_VAR_NAME identifier)"
            "(P_DELIMITER <period>)"
            "(P_SUBROUTINE_NAME mymethod))"
            "(P_DELIMITER <left_parenthesis>)"
            "(P_EXPRESSION_LIST "
            "(P_EXPRESSION (P_TERM (P_INTEGER_CONSTANT 1)))"
            "(P_DELIMITER <comma>)"
            "(P_EXPRESSION (P_TERM (P_INTEGER_CONSTANT 2))))"
            "(P_DELIMITER <right_parenthesis>))))");
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

  SECTION("do statement #1")
  {
    // <do-statement> ::= "do" <subroutine-call> ";"
    strcpy(R.buffer, "do fn();");
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
            "(P_DO_STATEMENT "
            "(P_KEYWORD do)"
            "(P_SUBROUTINE_CALL "
            "(P_SUBROUTINE_CALL_SITE_BINDING "
            "(P_SUBROUTINE_NAME fn))"
            "(P_DELIMITER <left_parenthesis>)"
            "(P_EXPRESSION_LIST )"
            "(P_DELIMITER <right_parenthesis>))"
            "(P_DELIMITER <semicolon>)))");
  }

  SECTION("do statement #2")
  {
    // <do-statement> ::= "do" <subroutine-call> ";"
    strcpy(R.buffer, "do cname.fn(1, 2);");
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
            "(P_DO_STATEMENT "
            "(P_KEYWORD do)"
            "(P_SUBROUTINE_CALL "
            "(P_SUBROUTINE_CALL_SITE_BINDING "
            "(P_CLASS_OR_VAR_NAME cname)"
            "(P_DELIMITER <period>)"
            "(P_SUBROUTINE_NAME fn))"
            "(P_DELIMITER <left_parenthesis>)"
            "(P_EXPRESSION_LIST "
            "(P_EXPRESSION (P_TERM (P_INTEGER_CONSTANT 1)))"
            "(P_DELIMITER <comma>)"
            "(P_EXPRESSION (P_TERM (P_INTEGER_CONSTANT 2))))"
            "(P_DELIMITER <right_parenthesis>))"
            "(P_DELIMITER <semicolon>)))");
  }

  SECTION("while statement")
  {
    // <while-statement> ::= "while" "(" <expression> ")" "{" <statements> "}"
    strcpy(R.buffer, "while (true) {}");
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
            "(P_WHILE_STATEMENT "
            "(P_KEYWORD while)"
            "(P_DELIMITER <left_parenthesis>)"
            "(P_EXPRESSION (P_TERM (P_KEYWORD_CONSTANT true)))"
            "(P_DELIMITER <right_parenthesis>)"
            "(P_DELIMITER <left_brace>)"
            "(P_STATEMENT_LIST )"
            "(P_DELIMITER <right_brace>)))");
  }

  SECTION("non-empty while statement")
  {
    // <while-statement> ::= "while" "(" <expression> ")" "{" <statements> "}"
    strcpy(R.buffer, "while (true) { let i = true; }");
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
            "(P_WHILE_STATEMENT "
            "(P_KEYWORD while)"
            "(P_DELIMITER <left_parenthesis>)"
            "(P_EXPRESSION (P_TERM (P_KEYWORD_CONSTANT true)))"
            "(P_DELIMITER <right_parenthesis>)"
            "(P_DELIMITER <left_brace>)"
            "(P_STATEMENT_LIST "
            "(P_LET_STATEMENT "
            "(P_KEYWORD let)"
            "(P_SCALAR_VAR i)"
            "(P_OP <equal>)"
            "(P_EXPRESSION (P_TERM (P_KEYWORD_CONSTANT true)))"
            "(P_DELIMITER <semicolon>)))"
            "(P_DELIMITER <right_brace>)))");
  }

  SECTION("let scalar statement")
  {
    // <let-statement> ::= "let" <var-name> {"[" <expression> "]"}? "="
    //                     <expression> ";"
    strcpy(R.buffer, "let myvar = true;");
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
            "(P_LET_STATEMENT "
            "(P_KEYWORD let)"
            "(P_SCALAR_VAR myvar)"
            "(P_OP <equal>)"
            "(P_EXPRESSION (P_TERM (P_KEYWORD_CONSTANT true)))"
            "(P_DELIMITER <semicolon>)))");
  }

  SECTION("let array indexed statement")
  {
    // <let-statement> ::= "let" <var-name> {"[" <expression> "]"}? "="
    //                     <expression> ";"
    strcpy(R.buffer, "let myvar[1] = true;");
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
            "(P_LET_STATEMENT "
            "(P_KEYWORD let)"
            "(P_ARRAY_VAR myvar)"
            "(P_DELIMITER <left_bracket>)"
            "(P_EXPRESSION (P_TERM (P_INTEGER_CONSTANT 1)))"
            "(P_DELIMITER <right_bracket>)"
            "(P_OP <equal>)"
            "(P_EXPRESSION (P_TERM (P_KEYWORD_CONSTANT true)))"
            "(P_DELIMITER <semicolon>)))");
  }

  SECTION("if statement")
  {
    // <if-statement> ::= "if" "(" <expression ")" "{" <statements> "}"
    strcpy(R.buffer, "if (true) {}");
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
            "(P_IF_STATEMENT "
            "(P_KEYWORD if)"
            "(P_DELIMITER <left_parenthesis>)"
            "(P_EXPRESSION (P_TERM (P_KEYWORD_CONSTANT true)))"
            "(P_DELIMITER <right_parenthesis>)"
            "(P_DELIMITER <left_brace>)"
            "(P_STATEMENT_LIST )"
            "(P_DELIMITER <right_brace>)))");
  }

  SECTION("if-else statement")
  {
    // <if-statement> ::= "if" "(" <expression ")" "{" <statements> "}"
    //                    {"else" "{" <statements> "}"}?
    strcpy(R.buffer, "if (true) {} else {}");
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
            "(P_IF_STATEMENT "
            "(P_KEYWORD if)"
            "(P_DELIMITER <left_parenthesis>)"
            "(P_EXPRESSION (P_TERM (P_KEYWORD_CONSTANT true)))"
            "(P_DELIMITER <right_parenthesis>)"
            "(P_DELIMITER <left_brace>)"
            "(P_STATEMENT_LIST )"
            "(P_DELIMITER <right_brace>)"
            "(P_KEYWORD else)"
            "(P_DELIMITER <left_brace>)"
            "(P_STATEMENT_LIST )"
            "(P_DELIMITER <right_brace>)))");
  }

  SECTION("assignment from function return")
  {
    // <class> ::= "class" <class-name> "{" {<class-var-decl>}*
    //             {<subroutine-decl>}* "}"
    strcpy(R.buffer, "let a = fn();");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_let_statement();
    REQUIRE(parsetree_node);

    // verify tree structure using strings
    stringstream ss;
    ss << *parsetree_node;
    REQUIRE(ss.str() ==
            "(P_LET_STATEMENT "
            "(P_KEYWORD let)"
            "(P_SCALAR_VAR a)"
            "(P_OP <equal>)"
            "(P_EXPRESSION (P_TERM "
            "(P_SUBROUTINE_CALL "
            "(P_SUBROUTINE_CALL_SITE_BINDING "
            "(P_SUBROUTINE_NAME fn))"
            "(P_DELIMITER <left_parenthesis>)"
            "(P_EXPRESSION_LIST )"
            "(P_DELIMITER <right_parenthesis>))))"
            "(P_DELIMITER <semicolon>))");
  }
}

SCENARIO("program low-level structures")
{
  test::MockReader R;

  SECTION("empty subroutine-body structure")
  {
    // <subroutine-body> ::= "{" {<var-decl>}* <statements> "}"
    strcpy(R.buffer, "{}");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_subroutine_body();
    REQUIRE(parsetree_node);

    // verify tree structure using strings
    stringstream ss;
    ss << *parsetree_node;
    REQUIRE(ss.str() ==
            "(P_SUBROUTINE_BODY "
            "(P_DELIMITER <left_brace>)"
            "(P_STATEMENT_LIST )"
            "(P_DELIMITER <right_brace>))");
  }

  SECTION("one variable declaration")
  {
    // <var-decl> ::= "var" <type> <var-name> {"," <var-name>}* ";"
    strcpy(R.buffer, "var int a;");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_variable_decl_block();
    REQUIRE(parsetree_node);

    // verify tree structure using strings
    stringstream ss;
    ss << *parsetree_node;
    REQUIRE(ss.str() ==
            "(P_VAR_DECL_BLOCK "
            "(P_VAR_DECL_STATEMENT "
            "(P_KEYWORD var)"
            "(P_VARIABLE_DECL "
            "(P_VARIABLE_TYPE integer)"
            "(P_VARIABLE_LIST "
            "(P_VARIABLE_NAME a)))"
            "(P_DELIMITER <semicolon>)))");
  }

  SECTION("multiple variable declaration")
  {
    // <var-decl> ::= "var" <type> <var-name> {"," <var-name>}* ";"
    strcpy(R.buffer, "var myclass a, b;");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_variable_decl_block();
    REQUIRE(parsetree_node);

    // verify tree structure using strings
    stringstream ss;
    ss << *parsetree_node;
    REQUIRE(ss.str() ==
            "(P_VAR_DECL_BLOCK "
            "(P_VAR_DECL_STATEMENT "
            "(P_KEYWORD var)"
            "(P_VARIABLE_DECL "
            "(P_VARIABLE_TYPE myclass)"
            "(P_VARIABLE_LIST "
            "(P_VARIABLE_NAME a)"
            "(P_DELIMITER <comma>)"
            "(P_VARIABLE_NAME b)))"
            "(P_DELIMITER <semicolon>)))");
  }

  SECTION("multi-line variable declaration")
  {
    // <var-decl> ::= "var" <type> <var-name> {"," <var-name>}* ";"
    strcpy(R.buffer, "var int a; var char b;");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_variable_decl_block();
    REQUIRE(parsetree_node);

    // verify tree structure using strings
    stringstream ss;
    ss << *parsetree_node;
    REQUIRE(ss.str() ==
            "(P_VAR_DECL_BLOCK "
            "(P_VAR_DECL_STATEMENT "
            "(P_KEYWORD var)"
            "(P_VARIABLE_DECL "
            "(P_VARIABLE_TYPE integer)"
            "(P_VARIABLE_LIST "
            "(P_VARIABLE_NAME a)))"
            "(P_DELIMITER <semicolon>))"
            "(P_VAR_DECL_STATEMENT "
            "(P_KEYWORD var)"
            "(P_VARIABLE_DECL "
            "(P_VARIABLE_TYPE character)"
            "(P_VARIABLE_LIST "
            "(P_VARIABLE_NAME b)))"
            "(P_DELIMITER <semicolon>)))");
  }

  SECTION("subroutine-body with empty statements structure")
  {
    // <subroutine-body> ::= "{" {<var-decl>}* <statements> "}"
    strcpy(R.buffer, "{var char a;}");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_subroutine_body();
    REQUIRE(parsetree_node);

    // verify tree structure using strings
    stringstream ss;
    ss << *parsetree_node;
    REQUIRE(ss.str() ==
            "(P_SUBROUTINE_BODY "
            "(P_DELIMITER <left_brace>)"
            "(P_VAR_DECL_BLOCK "
            "(P_VAR_DECL_STATEMENT "
            "(P_KEYWORD var)"
            "(P_VARIABLE_DECL "
            "(P_VARIABLE_TYPE character)"
            "(P_VARIABLE_LIST "
            "(P_VARIABLE_NAME a)))"
            "(P_DELIMITER <semicolon>)))"
            "(P_STATEMENT_LIST )"
            "(P_DELIMITER <right_brace>))");
  }

  SECTION("non-empty subroutine-body structure")
  {
    // <subroutine-body> ::= "{" {<var-decl>}* <statements> "}"
    strcpy(R.buffer, "{var boolean a; let a = true; }");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_subroutine_body();
    REQUIRE(parsetree_node);

    // verify tree structure using strings
    stringstream ss;
    ss << *parsetree_node;
    REQUIRE(ss.str() ==
            "(P_SUBROUTINE_BODY "
            "(P_DELIMITER <left_brace>)"
            "(P_VAR_DECL_BLOCK "
            "(P_VAR_DECL_STATEMENT "
            "(P_KEYWORD var)"
            "(P_VARIABLE_DECL "
            "(P_VARIABLE_TYPE boolean)"
            "(P_VARIABLE_LIST "
            "(P_VARIABLE_NAME a)))"
            "(P_DELIMITER <semicolon>)))"
            "(P_STATEMENT_LIST "
            "(P_LET_STATEMENT "
            "(P_KEYWORD let)"
            "(P_SCALAR_VAR a)"
            "(P_OP <equal>)"
            "(P_EXPRESSION (P_TERM (P_KEYWORD_CONSTANT true)))"
            "(P_DELIMITER <semicolon>)))"
            "(P_DELIMITER <right_brace>))");
  }

  SECTION("one builtin type parameter structure")
  {
    // <parameter-list> ::= {(<type> <var-name>) {"," <type> <var-name>}*}?
    strcpy(R.buffer, "boolean a");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_parameter_list();
    REQUIRE(parsetree_node);

    // verify tree structure using strings
    stringstream ss;
    ss << *parsetree_node;
    REQUIRE(ss.str() ==
            "(P_PARAMETER_LIST "
            "(P_VARIABLE_TYPE boolean)"
            "(P_VARIABLE_NAME a))");
  }

  SECTION("one class type parameter structure")
  {
    // <parameter-list> ::= {(<type> <var-name>) {"," <type> <var-name>}*}?
    strcpy(R.buffer, "MyClass a");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_parameter_list();
    REQUIRE(parsetree_node);

    // verify tree structure using strings
    stringstream ss;
    ss << *parsetree_node;
    REQUIRE(ss.str() ==
            "(P_PARAMETER_LIST "
            "(P_VARIABLE_TYPE MyClass)"
            "(P_VARIABLE_NAME a))");
  }

  SECTION("multiple parameter structure")
  {
    // <parameter-list> ::= {(<type> <var-name>) {"," <type> <var-name>}*}?
    strcpy(R.buffer, "boolean a, int b, char c, ClassName d");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_parameter_list();
    REQUIRE(parsetree_node);

    // verify tree structure using strings
    stringstream ss;
    ss << *parsetree_node;
    REQUIRE(ss.str() ==
            "(P_PARAMETER_LIST "
            "(P_VARIABLE_TYPE boolean)"
            "(P_VARIABLE_NAME a)"
            "(P_DELIMITER <comma>)"
            "(P_VARIABLE_TYPE integer)"
            "(P_VARIABLE_NAME b)"
            "(P_DELIMITER <comma>)"
            "(P_VARIABLE_TYPE character)"
            "(P_VARIABLE_NAME c)"
            "(P_DELIMITER <comma>)"
            "(P_VARIABLE_TYPE ClassName)"
            "(P_VARIABLE_NAME d))");
  }
}

SCENARIO("program high-level structures")
{
  test::MockReader R;

  SECTION("empty function decl")
  {
    // <subroutine-decl> ::= ("constructor" | "function" | "method")
    //                       ("void" | <type>) <subroutine-name>
    //                       "(" <parameter-list> ")" <subroutine-body>
    strcpy(R.buffer, "function int MySub() {}");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_subroutine_declaration();
    REQUIRE(parsetree_node);

    // verify tree structure using strings
    stringstream ss;
    ss << *parsetree_node;
    REQUIRE(ss.str() ==
            "(P_SUBROUTINE_DECL_BLOCK "
            "(P_SUBROUTINE_TYPE function)"
            "(P_RETURN_TYPE integer)"
            "(P_SUBROUTINE_NAME MySub)"
            "(P_DELIMITER <left_parenthesis>)"
            "(P_PARAMETER_LIST )"
            "(P_DELIMITER <right_parenthesis>)"
            "(P_SUBROUTINE_BODY "
            "(P_DELIMITER <left_brace>)"
            "(P_STATEMENT_LIST )"
            "(P_DELIMITER <right_brace>)))");
  }

  SECTION("empty method decl")
  {
    // <subroutine-decl> ::= ("constructor" | "function" | "method")
    //                       ("void" | <type>) <subroutine-name>
    //                       "(" <parameter-list> ")" <subroutine-body>
    strcpy(R.buffer, "method void MySub() {}");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_subroutine_declaration();
    REQUIRE(parsetree_node);

    // verify tree structure using strings
    stringstream ss;
    ss << *parsetree_node;
    REQUIRE(ss.str() ==
            "(P_SUBROUTINE_DECL_BLOCK "
            "(P_SUBROUTINE_TYPE method)"
            "(P_RETURN_TYPE void)"
            "(P_SUBROUTINE_NAME MySub)"
            "(P_DELIMITER <left_parenthesis>)"
            "(P_PARAMETER_LIST )"
            "(P_DELIMITER <right_parenthesis>)"
            "(P_SUBROUTINE_BODY "
            "(P_DELIMITER <left_brace>)"
            "(P_STATEMENT_LIST )"
            "(P_DELIMITER <right_brace>)))");
  }

  SECTION("non-empty subroutine decl")
  {
    // <subroutine-decl> ::= ("constructor" | "function" | "method")
    //                       ("void" | <type>) <subroutine-name>
    //                       "(" <parameter-list> ")" <subroutine-body>
    strcpy(R.buffer, "constructor void MySub(int a, int b) {let a = b;}");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_subroutine_declaration();
    REQUIRE(parsetree_node);

    // verify tree structure using strings
    stringstream ss;
    ss << *parsetree_node;
    REQUIRE(ss.str() ==
            // clang-format off
            "(P_SUBROUTINE_DECL_BLOCK "
            "(P_SUBROUTINE_TYPE constructor)"
            "(P_RETURN_TYPE void)"
            "(P_SUBROUTINE_NAME MySub)"
            "(P_DELIMITER <left_parenthesis>)"
              "(P_PARAMETER_LIST "
              "(P_VARIABLE_TYPE integer)"
              "(P_VARIABLE_NAME a)"
              "(P_DELIMITER <comma>)"
              "(P_VARIABLE_TYPE integer)"
              "(P_VARIABLE_NAME b))"
            "(P_DELIMITER <right_parenthesis>)"
            "(P_SUBROUTINE_BODY "
            "(P_DELIMITER <left_brace>)"
              "(P_STATEMENT_LIST "
              "(P_LET_STATEMENT "
              "(P_KEYWORD let)"
              "(P_SCALAR_VAR a)"
              "(P_OP <equal>)"
              "(P_EXPRESSION (P_TERM (P_SCALAR_VAR b)))"
              "(P_DELIMITER <semicolon>)))"
            "(P_DELIMITER <right_brace>)))");
    // clang-format on
  }

  SECTION("one class-var decl")
  {
    // <class-var-decl> ::= ("static" | "field") <type> <var-name> {","
    //                      <var-name>}* ";"
    strcpy(R.buffer, "static int a;");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_class_variable_decl_block();
    REQUIRE(parsetree_node);

    // verify tree structure using strings
    stringstream ss;
    ss << *parsetree_node;
    REQUIRE(ss.str() ==
            "(P_CLASSVAR_DECL_BLOCK "
            "(P_CLASSVAR_DECL_STATEMENT "
            "(P_CLASSVAR_SCOPE static)"
            "(P_VARIABLE_DECL "
            "(P_VARIABLE_TYPE integer)"
            "(P_VARIABLE_LIST "
            "(P_VARIABLE_NAME a)))"
            "(P_DELIMITER <semicolon>)))");
  }

  SECTION("multiple class-var decl")
  {
    // <class-var-decl> ::= ("static" | "field") <type> <var-name> {","
    //                      <var-name>}* ";"
    strcpy(R.buffer, "field boolean a, b;");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_class_variable_decl_block();
    REQUIRE(parsetree_node);

    // verify tree structure using strings
    stringstream ss;
    ss << *parsetree_node;
    REQUIRE(ss.str() ==
            "(P_CLASSVAR_DECL_BLOCK "
            "(P_CLASSVAR_DECL_STATEMENT "
            "(P_CLASSVAR_SCOPE field)"
            "(P_VARIABLE_DECL "
            "(P_VARIABLE_TYPE boolean)"
            "(P_VARIABLE_LIST "
            "(P_VARIABLE_NAME a)"
            "(P_DELIMITER <comma>)"
            "(P_VARIABLE_NAME b)))"
            "(P_DELIMITER <semicolon>)))");
  }

  SECTION("empty class decl")
  {
    // <class> ::= "class" <class-name> "{" {<class-var-decl>}*
    //             {<subroutine-decl>}* "}"
    strcpy(R.buffer, "class MyClass {}");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_class();
    REQUIRE(parsetree_node);

    // verify tree structure using strings
    stringstream ss;
    ss << *parsetree_node;
    REQUIRE(ss.str() ==
            "(P_CLASS_DECL_BLOCK "
            "(P_KEYWORD class)"
            "(P_CLASS_NAME MyClass)"
            "(P_DELIMITER <left_brace>)"
            "(P_DELIMITER <right_brace>))");
  }

  SECTION("invalid class decl")
  {
    // <class> ::= "class" <class-name> "{" {<class-var-decl>}*
    //             {<subroutine-decl>}* "}"
    strcpy(R.buffer, "class myclass {}");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    REQUIRE_THROWS(T.parse_class());
  }

  SECTION("class decl")
  {
    // <class> ::= "class" <class-name> "{" {<class-var-decl>}*
    //             {<subroutine-decl>}* "}"
    strcpy(R.buffer,
           "class MyClass { static char c; function int MySub() {} }");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_class();
    REQUIRE(parsetree_node);

    // verify tree structure using strings
    stringstream ss;
    ss << *parsetree_node;
    REQUIRE(ss.str() ==
            // clang-format off
            "(P_CLASS_DECL_BLOCK "
            "(P_KEYWORD class)"
            "(P_CLASS_NAME MyClass)"
            "(P_DELIMITER <left_brace>)"
            "(P_CLASSVAR_DECL_BLOCK "
              "(P_CLASSVAR_DECL_STATEMENT "
                "(P_CLASSVAR_SCOPE static)"
                "(P_VARIABLE_DECL "
                "(P_VARIABLE_TYPE character)"
                "(P_VARIABLE_LIST "
                "(P_VARIABLE_NAME c)))"
                "(P_DELIMITER <semicolon>)))"
            "(P_SUBROUTINE_DECL_BLOCK "
              "(P_SUBROUTINE_TYPE function)"
              "(P_RETURN_TYPE integer)"
              "(P_SUBROUTINE_NAME MySub)"
              "(P_DELIMITER <left_parenthesis>)"
              "(P_PARAMETER_LIST )"
              "(P_DELIMITER <right_parenthesis>)"
              "(P_SUBROUTINE_BODY "
              "(P_DELIMITER <left_brace>)"
              "(P_STATEMENT_LIST )"
              "(P_DELIMITER <right_brace>)))"
            "(P_DELIMITER <right_brace>))");
    // clang-format on
  }

  SECTION("class two var decl")
  {
    // <class> ::= "class" <class-name> "{" {<class-var-decl>}*
    //             {<subroutine-decl>}* "}"
    strcpy(R.buffer,
           "class MyClass { field int a; field int b; method void fn() {} }");
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(tokens);
    auto parsetree_node = T.parse_class();
    REQUIRE(parsetree_node);

    // verify tree structure using strings
    stringstream ss;
    ss << *parsetree_node;
    REQUIRE(ss.str() ==
            // clang-format off
            "(P_CLASS_DECL_BLOCK "
            "(P_KEYWORD class)"
            "(P_CLASS_NAME MyClass)"
            "(P_DELIMITER <left_brace>)"
            "(P_CLASSVAR_DECL_BLOCK "
              "(P_CLASSVAR_DECL_STATEMENT "
                "(P_CLASSVAR_SCOPE field)"
                "(P_VARIABLE_DECL "
                "(P_VARIABLE_TYPE integer)"
                "(P_VARIABLE_LIST "
                "(P_VARIABLE_NAME a)))"
                "(P_DELIMITER <semicolon>))"
              "(P_CLASSVAR_DECL_STATEMENT "
                "(P_CLASSVAR_SCOPE field)"
                "(P_VARIABLE_DECL "
                "(P_VARIABLE_TYPE integer)"
                "(P_VARIABLE_LIST "
                "(P_VARIABLE_NAME b)))"
                "(P_DELIMITER <semicolon>)))"
            "(P_SUBROUTINE_DECL_BLOCK "
              "(P_SUBROUTINE_TYPE method)"
              "(P_RETURN_TYPE void)"
              "(P_SUBROUTINE_NAME fn)"
              "(P_DELIMITER <left_parenthesis>)"
              "(P_PARAMETER_LIST )"
              "(P_DELIMITER <right_parenthesis>)"
              "(P_SUBROUTINE_BODY "
              "(P_DELIMITER <left_brace>)"
              "(P_STATEMENT_LIST )"
              "(P_DELIMITER <right_brace>)))"
            "(P_DELIMITER <right_brace>))");
    // clang-format on
  }
}
