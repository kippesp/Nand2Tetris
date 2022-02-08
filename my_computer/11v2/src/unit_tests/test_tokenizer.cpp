#include "common.h"

/////////////////////////////////////////////////////////////////////////////
// Tests for Jack Symbols
/////////////////////////////////////////////////////////////////////////////

using namespace jfcl;

SCENARIO("Tokenizer baseline checks")
{
  SECTION("tokenizer")
  {
    TextReader R("+");
    JackTokenizer T(R);

    auto expected_parsed_tokens = std::vector {
        TokenValue_t::J_PLUS,  // '+'
        TokenValue_t::J_EOF,
    };

    auto tokens = T.parse_tokens();

    REQUIRE(tokens.size() == expected_parsed_tokens.size());

    REQUIRE(tokens[0].get().value_enum == TokenValue_t::J_PLUS);

    REQUIRE(tokens[1].get().value_enum == TokenValue_t::J_EOF);
  }
}

SCENARIO("Tokenize Jack symbols")
{
  TextReader R(" { } () [ ] .  , ; + - * / & | < > = ~ ");
  JackTokenizer T(R);

  auto expected_parsed_tokens = std::vector {
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

    REQUIRE(tokens.size() == expected_parsed_tokens.size());

    for (decltype(tokens.size()) i = 0; i < tokens.size() - 1; i++)
    {
      REQUIRE(tokens[i].get().value_enum == expected_parsed_tokens[i]);
    }
  }
}

/////////////////////////////////////////////////////////////////////////////
// Tests for Jack Keywords
/////////////////////////////////////////////////////////////////////////////

SCENARIO("Tokenize Jack keywords")
{
  TextReader R(
      " if let  char this void false while return constructor\n"
      "function boolean static method field class true null else var int do");

  JackTokenizer T(R);

  auto expected_parsed_tokens = std::vector {
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

    REQUIRE(tokens.size() == expected_parsed_tokens.size());

    for (decltype(tokens.size()) i = 0; i < tokens.size() - 1; i++)
    {
      REQUIRE(tokens[i].get().value_enum == expected_parsed_tokens[i]);
    }
  }
}

SCENARIO("Tokenize integers")
{
  SECTION("single integers")
  {
    TextReader R(" 1  5 9 ");
    JackTokenizer T(R);

    auto expected_parsed_integers = std::vector {"1", "5", "9"};
    auto tokens = T.parse_tokens();

    REQUIRE(tokens.size() == expected_parsed_integers.size() + 1);

    for (decltype(tokens.size()) i = 0; i < tokens.size() - 1; i++)
    {
      REQUIRE(tokens[i].get().value_enum == TokenValue_t::J_INTEGER_CONSTANT);
      REQUIRE(tokens[i].get().value_str == expected_parsed_integers[i]);
    }
  }

  SECTION("long integers")
  {
    TextReader R(" 0987654321  5555 99 ");
    JackTokenizer T(R);

    auto expected_parsed_integers = std::vector {"0987654321", "5555", "99"};
    auto tokens = T.parse_tokens();

    REQUIRE(tokens.size() == expected_parsed_integers.size() + 1);

    for (decltype(tokens.size()) i = 0; i < tokens.size() - 1; i++)
    {
      REQUIRE(tokens[i].get().value_enum == TokenValue_t::J_INTEGER_CONSTANT);
      REQUIRE(tokens[i].get().value_str == expected_parsed_integers[i]);
    }
  }

  SECTION("simple statement")
  {
    TextReader R(" 10+5 * -2 ");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    REQUIRE(tokens.size() == 7);

    REQUIRE(tokens[0].get().value_enum == TokenValue_t::J_INTEGER_CONSTANT);
    REQUIRE(tokens[0].get().value_str == "10");

    REQUIRE(tokens[1].get().value_enum == TokenValue_t::J_PLUS);

    REQUIRE(tokens[2].get().value_enum == TokenValue_t::J_INTEGER_CONSTANT);
    REQUIRE(tokens[2].get().value_str == "5");

    REQUIRE(tokens[3].get().value_enum == TokenValue_t::J_ASTERISK);

    REQUIRE(tokens[4].get().value_enum == TokenValue_t::J_MINUS);

    REQUIRE(tokens[5].get().value_enum == TokenValue_t::J_INTEGER_CONSTANT);
    REQUIRE(tokens[5].get().value_str == "2");

    REQUIRE(tokens[6].get().value_enum == TokenValue_t::J_EOF);
    REQUIRE(tokens[6].get().value_str == "( eof )");
  }
}

SCENARIO("Tokenize identifiers")
{
  SECTION("multiple identifiers")
  {
    TextReader R("c abcd bbbb _AazZ _0a __9z ");
    JackTokenizer T(R);

    auto expected_parsed_identifiers =
        std::vector {"c", "abcd", "bbbb", "_AazZ", "_0a", "__9z"};
    auto tokens = T.parse_tokens();

    REQUIRE(tokens.size() == expected_parsed_identifiers.size() + 1);

    for (decltype(tokens.size()) i = 0; i < tokens.size() - 1; i++)
    {
      REQUIRE(tokens[i].get().value_enum == TokenValue_t::J_IDENTIFIER);
      REQUIRE(tokens[i].get().value_str == expected_parsed_identifiers[i]);
    }
  }

  SECTION("uppercase identifiers")
  {
    TextReader R("CLASS METHOD ");
    JackTokenizer T(R);

    auto expected_parsed_identifiers = std::vector {"CLASS", "METHOD"};
    auto tokens = T.parse_tokens();

    REQUIRE(tokens.size() == expected_parsed_identifiers.size() + 1);

    for (decltype(tokens.size()) i = 0; i < tokens.size() - 1; i++)
    {
      REQUIRE(tokens[i].get().value_enum == TokenValue_t::J_IDENTIFIER);
      REQUIRE(tokens[i].get().value_str == expected_parsed_identifiers[i]);
    }
  }

  SECTION("identifiers containing reserved strings")
  {
    TextReader R("ifclass _let char_ while1");
    JackTokenizer T(R);

    auto expected_parsed_identifiers =
        std::vector {"ifclass", "_let", "char_", "while1"};
    auto tokens = T.parse_tokens();

    REQUIRE(tokens.size() == expected_parsed_identifiers.size() + 1);

    for (decltype(tokens.size()) i = 0; i < tokens.size() - 1; i++)
    {
      REQUIRE(tokens[i].get().value_enum == TokenValue_t::J_IDENTIFIER);
      REQUIRE(tokens[i].get().value_str == expected_parsed_identifiers[i]);
    }
  }

  SECTION("miscellaneous")
  {
    TextReader R("main\n\r\n\r\tclass");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    REQUIRE(tokens.size() == 3);

    REQUIRE(tokens[0].get().value_enum == TokenValue_t::J_IDENTIFIER);
    REQUIRE(tokens[0].get().value_str == "main");

    REQUIRE(tokens[1].get().value_enum == TokenValue_t::J_CLASS);
  }
}

SCENARIO("Tokenize string")
{
  SECTION("simple strings")
  {
    TextReader R(
        "\"test class \" "
        "\"\" "
        "\" \"");
    JackTokenizer T(R);

    auto expected_parsed_identifiers = std::vector {"test class ", "", " "};
    auto tokens = T.parse_tokens();

    REQUIRE(tokens.size() == expected_parsed_identifiers.size() + 1);

    for (decltype(tokens.size()) i = 0; i < tokens.size() - 1; i++)
    {
      REQUIRE(tokens[i].get().value_enum == TokenValue_t::J_STRING_CONSTANT);
      REQUIRE(tokens[i].get().value_str == expected_parsed_identifiers[i]);
    }
  }

  SECTION("strings with delimited characters")
  {
    std::string input_string = R":(("What is \" wrong \" here?")):";
    input_string += R":(("This \\ is odd")):";
    TextReader R(input_string.data());
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    REQUIRE(tokens.size() == 7);

    REQUIRE(tokens[0].get().value_enum == TokenValue_t::J_LEFT_PARENTHESIS);
    REQUIRE(tokens[1].get().value_enum == TokenValue_t::J_STRING_CONSTANT);
    REQUIRE(tokens[2].get().value_enum == TokenValue_t::J_RIGHT_PARENTHESIS);
    REQUIRE(tokens[1].get().value_str == R":(What is " wrong " here?):");

    REQUIRE(tokens[3].get().value_enum == TokenValue_t::J_LEFT_PARENTHESIS);
    REQUIRE(tokens[4].get().value_enum == TokenValue_t::J_STRING_CONSTANT);
    REQUIRE(tokens[5].get().value_enum == TokenValue_t::J_RIGHT_PARENTHESIS);
    REQUIRE(tokens[4].get().value_str == R":(This \ is odd):");
  }
}

SCENARIO("Tokenize comments")
{
  SECTION("line comments")
  {
    TextReader R(
        "// comment1\n"
        "// comment2\x0d\x0a"
        "// if comment");
    JackTokenizer T(R);

    auto expected_parsed_identifiers =
        std::vector {"// comment1", "// comment2", "// if comment"};
    auto tokens = T.parse_tokens();

    REQUIRE(tokens.size() == expected_parsed_identifiers.size() + 1);

    for (decltype(tokens.size()) i = 0; i < tokens.size() - 1; i++)
    {
      REQUIRE(tokens[i].get().value_enum == TokenValue_t::J_COMMENT);
      REQUIRE(tokens[i].get().value_str == expected_parsed_identifiers[i]);
    }
  }

  SECTION("mixed comments")
  {
    TextReader R("class + // comment1\n");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    REQUIRE(tokens.size() == 4);

    REQUIRE(tokens[0].get().value_enum == TokenValue_t::J_CLASS);

    REQUIRE(tokens[1].get().value_enum == TokenValue_t::J_PLUS);

    REQUIRE(tokens[2].get().value_str == "// comment1");
    REQUIRE(tokens[2].get().value_enum == TokenValue_t::J_COMMENT);
  }

  SECTION("block comments")
  {
    TextReader R(
        "/* class */\n"
        "/**/ "
        "/*   */\n"
        "/** x */");
    JackTokenizer T(R);

    auto expected_parsed_identifiers =
        std::vector {"/* class */", "/**/", "/*   */", "/** x */"};

    auto tokens = T.parse_tokens();

    REQUIRE(tokens.size() == expected_parsed_identifiers.size() + 1);

    for (decltype(tokens.size()) i = 0; i < tokens.size() - 1; i++)
    {
      REQUIRE(tokens[i].get().value_enum == TokenValue_t::J_COMMENT);
      REQUIRE(tokens[i].get().value_str == expected_parsed_identifiers[i]);
    }
  }
}

SCENARIO("Tokenizer lexical issues")
{
  SECTION("unterminated block commment")
  {
    TextReader R("/* comment");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    REQUIRE(tokens.size() == 2);

    REQUIRE(tokens[0].get().value_str == "/* comment");
    REQUIRE(tokens[0].get().value_enum == TokenValue_t::J_UNDEFINED);

    REQUIRE(tokens[1].get().value_enum == TokenValue_t::J_EOF);
    REQUIRE(tokens[1].get().value_str == "( eof )");
  }

  SECTION("unterminated string")
  {
    TextReader R("\"string ");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    REQUIRE(tokens.size() == 2);

    REQUIRE(tokens[0].get().value_str == "string ");
    REQUIRE(tokens[0].get().value_enum == TokenValue_t::J_UNDEFINED);

    REQUIRE(tokens[1].get().value_enum == TokenValue_t::J_EOF);
    REQUIRE(tokens[1].get().value_str == "( eof )");
  }

  SECTION("empty file")
  {
    TextReader R("");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    REQUIRE(tokens.size() == 1);

    REQUIRE(tokens[0].get().value_enum == TokenValue_t::J_EOF);
    REQUIRE(tokens[0].get().value_str == "( eof )");
  }
}

SCENARIO("Token Comparisons")
{
  SECTION("equality/inequality")
  {
    TextReader R("+ +\n+ -");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    REQUIRE(tokens[0].get().value_enum == TokenValue_t::J_PLUS);
    REQUIRE(tokens[1].get().value_enum == TokenValue_t::J_PLUS);
    REQUIRE(tokens[2].get().value_enum == TokenValue_t::J_PLUS);

    REQUIRE(tokens[0].get() == tokens[0].get());
    REQUIRE(tokens[0].get() == tokens[1].get());
    REQUIRE(tokens[0].get() == tokens[2].get());
    REQUIRE(tokens[1].get() == tokens[2].get());

    REQUIRE(tokens[0].get().line_number == tokens[1].get().line_number);
    REQUIRE(tokens[0].get().line_number != tokens[2].get().line_number);

    REQUIRE(!(tokens[0].get() == tokens[3].get()));
    REQUIRE(tokens[0].get() != tokens[3].get());
  }
}
