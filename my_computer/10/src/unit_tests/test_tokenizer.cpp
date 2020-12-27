#include <string.h>

#include <vector>

#include "catch.hpp"
#include "mock_reader.h"
#include "tokenizer/jack_tokenizer.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// Tests for Jack Symbols
/////////////////////////////////////////////////////////////////////////////

SCENARIO("Tokenizer baseline checks")
{
  test::MockReader R;

  SECTION("tokenizer")
  {
    strcpy(R.buffer, "+");
    JackTokenizer T(R);

    auto expected_parsed_tokens = vector{
        TokenValue_t::J_PLUS,  // '+'
        TokenValue_t::J_EOF,
    };

    auto tokens = T.parse_tokens();

    REQUIRE(tokens->size() == expected_parsed_tokens.size());

    REQUIRE((*tokens)[0].type == TokenType_t::J_SYMBOL);
    REQUIRE((*tokens)[0].value_enum == TokenValue_t::J_PLUS);

    REQUIRE((*tokens)[1].type == TokenType_t::J_INTERNAL);
    REQUIRE((*tokens)[1].value_enum == TokenValue_t::J_EOF);
  }
}

SCENARIO("Tokenize Jack symbols")
{
  test::MockReader R;
  strcpy(R.buffer, " { } () [ ] .  , ; + - * / & | < > = ~ ");
  JackTokenizer T(R);

  auto expected_parsed_tokens = vector{
      TokenValue_t::J_LEFT_BRACE,         // '{'
      TokenValue_t::J_RIGHT_BRACE,        // '}'
      TokenValue_t::J_LEFT_PARENTHESIS,   // '('
      TokenValue_t::J_RIGHT_PARENTHESIS,  // ')'
      TokenValue_t::J_LEFT_BRACKET,       // '['
      TokenValue_t::J_RIGHT_BRACKET,      // ']'
      TokenValue_t::J_PERIOD,             // '.'
      TokenValue_t::J_COMMA,              // ','
      TokenValue_t::J_SEMICOLON,          // ';'
      TokenValue_t::J_PLUS,               // '+'
      TokenValue_t::J_MINUS,              // '-'
      TokenValue_t::J_ASTERISK,           // '*'
      TokenValue_t::J_DIVIDE,             // '/'
      TokenValue_t::J_AMPERSAND,          // '&'
      TokenValue_t::J_VBAR,               // '|'
      TokenValue_t::J_LESS_THAN,          // '<'
      TokenValue_t::J_GREATER_THAN,       // '>'
      TokenValue_t::J_EQUAL,              // '='
      TokenValue_t::J_TILDE,              // '~'
      TokenValue_t::J_EOF,
  };

  SECTION("tokenizer")
  {
    auto tokens = T.parse_tokens();

    REQUIRE(tokens->size() == expected_parsed_tokens.size());

    for (decltype(tokens->size()) i = 0; i < tokens->size() - 1; i++)
    {
      REQUIRE((*tokens)[i].type == TokenType_t::J_SYMBOL);
      REQUIRE((*tokens)[i].value_enum == expected_parsed_tokens[i]);
    }
  }
}

/////////////////////////////////////////////////////////////////////////////
// Tests for Jack Keywords
/////////////////////////////////////////////////////////////////////////////

SCENARIO("Tokenize Jack keywords")
{
  test::MockReader R;
  strcpy(
      R.buffer,
      " if let  char this void false while return constructor\n"
      "function boolean static method field class true null else var int do");

  JackTokenizer T(R);

  auto expected_parsed_tokens = vector{
      // clang-format off
      TokenValue_t::J_IF,
      TokenValue_t::J_LET,
      TokenValue_t::J_CHAR,
      TokenValue_t::J_THIS,
      TokenValue_t::J_VOID,
      TokenValue_t::J_FALSE,
      TokenValue_t::J_WHILE,
      TokenValue_t::J_RETURN,
      TokenValue_t::J_CONSTRUCTOR,
      TokenValue_t::J_FUNCTION,
      TokenValue_t::J_BOOLEAN,
      TokenValue_t::J_STATIC,
      TokenValue_t::J_METHOD,
      TokenValue_t::J_FIELD,
      TokenValue_t::J_CLASS,
      TokenValue_t::J_TRUE,
      TokenValue_t::J_NULL,
      TokenValue_t::J_ELSE,
      TokenValue_t::J_VAR,
      TokenValue_t::J_INT,
      TokenValue_t::J_DO,
      TokenValue_t::J_EOF,
      // clang-format on
  };

  SECTION("tokenizer")
  {
    auto tokens = T.parse_tokens();

    REQUIRE(tokens->size() == expected_parsed_tokens.size());

    for (decltype(tokens->size()) i = 0; i < tokens->size() - 1; i++)
    {
      REQUIRE((*tokens)[i].type == TokenType_t::J_KEYWORD);
      REQUIRE((*tokens)[i].value_enum == expected_parsed_tokens[i]);
    }
  }
}

SCENARIO("Tokenize integers")
{
  test::MockReader R;

  SECTION("single integers")
  {
    strcpy(R.buffer, " 1  5 9 ");
    auto expected_parsed_integers = vector{"1", "5", "9"};

    JackTokenizer T(R);
    auto tokens = T.parse_tokens();

    REQUIRE(tokens->size() == expected_parsed_integers.size() + 1);

    for (decltype(tokens->size()) i = 0; i < tokens->size() - 1; i++)
    {
      REQUIRE((*tokens)[i].type == TokenType_t::J_INTEGER_CONSTANT);
      REQUIRE((*tokens)[i].value_enum == TokenValue_t::J_NON_ENUM);
      REQUIRE((*tokens)[i].value_str == expected_parsed_integers[i]);
    }
  }

  SECTION("long integers")
  {
    strcpy(R.buffer, " 0987654321  5555 99 ");
    auto expected_parsed_integers = vector{"0987654321", "5555", "99"};

    JackTokenizer T(R);
    auto tokens = T.parse_tokens();

    REQUIRE(tokens->size() == expected_parsed_integers.size() + 1);

    for (decltype(tokens->size()) i = 0; i < tokens->size() - 1; i++)
    {
      REQUIRE((*tokens)[i].type == TokenType_t::J_INTEGER_CONSTANT);
      REQUIRE((*tokens)[i].value_enum == TokenValue_t::J_NON_ENUM);
      REQUIRE((*tokens)[i].value_str == expected_parsed_integers[i]);
    }
  }

  SECTION("simple statement")
  {
    strcpy(R.buffer, " 10+5 * -2 ");

    JackTokenizer T(R);
    auto tokens = T.parse_tokens();

    REQUIRE(tokens->size() == 7);

    REQUIRE((*tokens)[0].type == TokenType_t::J_INTEGER_CONSTANT);
    REQUIRE((*tokens)[0].value_str == "10");

    REQUIRE((*tokens)[1].type == TokenType_t::J_SYMBOL);
    REQUIRE((*tokens)[1].value_enum == TokenValue_t::J_PLUS);

    REQUIRE((*tokens)[2].type == TokenType_t::J_INTEGER_CONSTANT);
    REQUIRE((*tokens)[2].value_str == "5");

    REQUIRE((*tokens)[3].type == TokenType_t::J_SYMBOL);
    REQUIRE((*tokens)[3].value_enum == TokenValue_t::J_ASTERISK);

    REQUIRE((*tokens)[4].type == TokenType_t::J_SYMBOL);
    REQUIRE((*tokens)[4].value_enum == TokenValue_t::J_MINUS);

    REQUIRE((*tokens)[5].type == TokenType_t::J_INTEGER_CONSTANT);
    REQUIRE((*tokens)[5].value_str == "2");

    REQUIRE((*tokens)[6].type == TokenType_t::J_INTERNAL);
    REQUIRE((*tokens)[6].value_enum == TokenValue_t::J_EOF);
    REQUIRE((*tokens)[6].value_str == "( eof )");
  }
}

SCENARIO("Tokenize identifiers")
{
  test::MockReader R;

  SECTION("multiple identifiers")
  {
    strcpy(R.buffer, "c abcd bbbb _AazZ _0a __9z ");
    auto expected_parsed_identifiers =
        vector{"c", "abcd", "bbbb", "_AazZ", "_0a", "__9z"};

    JackTokenizer T(R);
    auto tokens = T.parse_tokens();

    REQUIRE(tokens->size() == expected_parsed_identifiers.size() + 1);

    for (decltype(tokens->size()) i = 0; i < tokens->size() - 1; i++)
    {
      REQUIRE((*tokens)[i].type == TokenType_t::J_IDENTIFIER);
      REQUIRE((*tokens)[i].value_enum == TokenValue_t::J_NON_ENUM);
      REQUIRE((*tokens)[i].value_str == expected_parsed_identifiers[i]);
    }
  }

  SECTION("uppercase identifiers")
  {
    strcpy(R.buffer, "CLASS METHOD ");
    auto expected_parsed_identifiers = vector{"CLASS", "METHOD"};

    JackTokenizer T(R);
    auto tokens = T.parse_tokens();

    REQUIRE(tokens->size() == expected_parsed_identifiers.size() + 1);

    for (decltype(tokens->size()) i = 0; i < tokens->size() - 1; i++)
    {
      REQUIRE((*tokens)[i].type == TokenType_t::J_IDENTIFIER);
      REQUIRE((*tokens)[i].value_enum == TokenValue_t::J_NON_ENUM);
      REQUIRE((*tokens)[i].value_str == expected_parsed_identifiers[i]);
    }
  }

  SECTION("identifiers containing reserved strings")
  {
    strcpy(R.buffer, "ifclass _let char_ while1");
    auto expected_parsed_identifiers =
        vector{"ifclass", "_let", "char_", "while1"};

    JackTokenizer T(R);
    auto tokens = T.parse_tokens();

    REQUIRE(tokens->size() == expected_parsed_identifiers.size() + 1);

    for (decltype(tokens->size()) i = 0; i < tokens->size() - 1; i++)
    {
      REQUIRE((*tokens)[i].type == TokenType_t::J_IDENTIFIER);
      REQUIRE((*tokens)[i].value_enum == TokenValue_t::J_NON_ENUM);
      REQUIRE((*tokens)[i].value_str == expected_parsed_identifiers[i]);
    }
  }

  SECTION("miscellaneous")
  {
    strcpy(R.buffer, "main\n\r\n\r\tclass");

    JackTokenizer T(R);
    auto tokens = T.parse_tokens();

    REQUIRE(tokens->size() == 3);

    REQUIRE((*tokens)[0].type == TokenType_t::J_IDENTIFIER);
    REQUIRE((*tokens)[0].value_enum == TokenValue_t::J_NON_ENUM);
    REQUIRE((*tokens)[0].value_str == "main");

    REQUIRE((*tokens)[1].type == TokenType_t::J_KEYWORD);
    REQUIRE((*tokens)[1].value_enum == TokenValue_t::J_CLASS);
  }
}

SCENARIO("Tokenize string")
{
  test::MockReader R;

  SECTION("simple strings")
  {
    strcpy(R.buffer,
           "\"test class \" "
           "\"\" "
           "\" \"");
    auto expected_parsed_identifiers = vector{"test class ", "", " "};

    JackTokenizer T(R);
    auto tokens = T.parse_tokens();

    REQUIRE(tokens->size() == expected_parsed_identifiers.size() + 1);

    for (decltype(tokens->size()) i = 0; i < tokens->size() - 1; i++)
    {
      REQUIRE((*tokens)[i].type == TokenType_t::J_STRING_CONSTANT);
      REQUIRE((*tokens)[i].value_enum == TokenValue_t::J_NON_ENUM);
      REQUIRE((*tokens)[i].value_str == expected_parsed_identifiers[i]);
    }
  }
}

SCENARIO("Tokenize comments")
{
  test::MockReader R;

  SECTION("line comments")
  {
    strcpy(R.buffer,
           "// comment1\n"
           "// comment2\x0d\x0a"
           "// if comment");
    auto expected_parsed_identifiers =
        vector{"// comment1", "// comment2", "// if comment"};

    JackTokenizer T(R);
    auto tokens = T.parse_tokens();

    REQUIRE(tokens->size() == expected_parsed_identifiers.size() + 1);

    for (decltype(tokens->size()) i = 0; i < tokens->size() - 1; i++)
    {
      REQUIRE((*tokens)[i].type == TokenType_t::J_INTERNAL);
      REQUIRE((*tokens)[i].value_enum == TokenValue_t::J_COMMENT);
      REQUIRE((*tokens)[i].value_str == expected_parsed_identifiers[i]);
    }
  }

  SECTION("mixed comments")
  {
    strcpy(R.buffer, "class + // comment1\n");

    JackTokenizer T(R);
    auto tokens = T.parse_tokens();

    REQUIRE(tokens->size() == 4);

    REQUIRE((*tokens)[0].type == TokenType_t::J_KEYWORD);
    REQUIRE((*tokens)[0].value_enum == TokenValue_t::J_CLASS);

    REQUIRE((*tokens)[1].type == TokenType_t::J_SYMBOL);
    REQUIRE((*tokens)[1].value_enum == TokenValue_t::J_PLUS);

    REQUIRE((*tokens)[2].type == TokenType_t::J_INTERNAL);
    REQUIRE((*tokens)[2].value_str == "// comment1");
    REQUIRE((*tokens)[2].value_enum == TokenValue_t::J_COMMENT);
  }

  SECTION("block comments")
  {
    strcpy(R.buffer,
           "/* class */\n"
           "/**/ "
           "/*   */\n"
           "/** x */");
    auto expected_parsed_identifiers =
        vector{"/* class */", "/**/", "/*   */", "/** x */"};

    JackTokenizer T(R);
    auto tokens = T.parse_tokens();

    REQUIRE(tokens->size() == expected_parsed_identifiers.size() + 1);

    for (decltype(tokens->size()) i = 0; i < tokens->size() - 1; i++)
    {
      REQUIRE((*tokens)[i].type == TokenType_t::J_INTERNAL);
      REQUIRE((*tokens)[i].value_enum == TokenValue_t::J_COMMENT);
      REQUIRE((*tokens)[i].value_str == expected_parsed_identifiers[i]);
    }
  }
}

SCENARIO("Tokenizer lexical issues")
{
  test::MockReader R;

  SECTION("unterminated block commment")
  {
    strcpy(R.buffer, "/* comment");

    JackTokenizer T(R);
    auto tokens = T.parse_tokens();

    REQUIRE(tokens->size() == 2);

    REQUIRE((*tokens)[0].type == TokenType_t::J_UNDEFINED);
    REQUIRE((*tokens)[0].value_str == "/* comment");
    REQUIRE((*tokens)[0].value_enum == TokenValue_t::J_NON_ENUM);

    REQUIRE((*tokens)[1].type == TokenType_t::J_INTERNAL);
    REQUIRE((*tokens)[1].value_enum == TokenValue_t::J_EOF);
    REQUIRE((*tokens)[1].value_str == "( eof )");
  }

  SECTION("unterminated string")
  {
    strcpy(R.buffer, "\"string ");

    JackTokenizer T(R);
    auto tokens = T.parse_tokens();

    REQUIRE(tokens->size() == 2);

    REQUIRE((*tokens)[0].type == TokenType_t::J_UNDEFINED);
    REQUIRE((*tokens)[0].value_str == "string ");
    REQUIRE((*tokens)[0].value_enum == TokenValue_t::J_NON_ENUM);

    REQUIRE((*tokens)[1].type == TokenType_t::J_INTERNAL);
    REQUIRE((*tokens)[1].value_enum == TokenValue_t::J_EOF);
    REQUIRE((*tokens)[1].value_str == "( eof )");
  }

  SECTION("empty file")
  {
    strcpy(R.buffer, "");

    JackTokenizer T(R);
    auto tokens = T.parse_tokens();

    REQUIRE(tokens->size() == 1);

    REQUIRE((*tokens)[0].type == TokenType_t::J_INTERNAL);
    REQUIRE((*tokens)[0].value_enum == TokenValue_t::J_EOF);
    REQUIRE((*tokens)[0].value_str == "( eof )");
  }
}
