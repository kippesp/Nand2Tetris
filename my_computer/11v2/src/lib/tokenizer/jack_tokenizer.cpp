// jack_tokenizer.cpp - converts jack code to tokenized list

#include "jack_tokenizer.h"

#include <cassert>
#include <sstream>
#include <string>

namespace jfcl {

using char_type = TextReader::char_type;

// convenience structure to build a helper vector
using TokenDescr_t = struct TokenDescr_s {
  std::string expected_str;
  TokenValue_t token_value;
  std::string dbg_str;
};

bool JackTokenizer::valid_identifier_char(char ch)
{
  bool r = false;
  r |= (ch == '_');
  r |= (ch >= '0') && (ch <= '9');
  ch |= 0x20;
  r |= (ch >= 'a') && (ch <= 'z');
  return r;
}

const Tokens_t JackTokenizer::parse_tokens()
{
  for (bool done = false; !done;)
  {
    auto token = get_next_token();

    const auto& added_token =
        tokens.emplace_back(std::make_unique<JackToken>(token));

    token_refs.emplace_back(*added_token);
    done = (token.value_enum == TokenValue_t::J_EOF);
  }

  return token_refs;
}

JackToken JackTokenizer::get_next_token()
{
  char ch {0};

  // Read one character, skipping over whitespace
  for (bool done = false; !done;)
  {
    ch = reader.read();
    if ((ch == -1) || (ch == '\0'))
    {
      return JackToken(TokenValue_t::J_EOF, "( eof )",
                       reader.get_current_line_number());
    }

    // skip over excess (non-token separators) spaces
    if (ch == ' ')
    {
      continue;
    }

    if (ch == '\n')
    {
      continue;
    }
    if (ch == '\r')
    {
      continue;
    }
    if (ch == '\t')
    {
      continue;
    }
    if (ch == '\0')
    {
      return JackToken(TokenValue_t::J_EOF, "( eof )",
                       reader.get_current_line_number());
    }

    done = true;
  }

  JackToken token;

  // PARSE FOR COMMENTS
  if ((ch == '/') && (reader.peek() == '/'))
  {
    // Check for line comments
    token = get_line_comment_token(ch);

    assert(token.value_enum == TokenValue_t::J_COMMENT);

    if (token.value_enum == TokenValue_t::J_COMMENT)
    {
      return token;
    }

    assert(false && "Should not get here");
  }
  else if ((ch == '/') && (reader.peek() == '*'))
  {
    // Check for block comments
    token = get_block_comment_token(ch);

    if (token.value_enum == TokenValue_t::J_COMMENT)
    {
      return token;
    }

    // An unterminated /* is malformed.
    assert(token.value_enum == TokenValue_t::J_UNDEFINED);
    return token;
  }

  // PARSE FOR STRING
  if (ch == '"')
  {
    token = get_string_token(ch);

    if (token.value_enum == TokenValue_t::J_STRING_CONSTANT)
    {
      return token;
    }

    // An unterminated " is malformed as is a string containing a newline
    assert(token.value_enum == TokenValue_t::J_UNDEFINED);
    return token;
  }

  // PARSE FOR INTEGER
  if ((ch >= '0') && (ch <= '9'))
  {
    token = get_integer_token(ch);

    if (token.value_enum == TokenValue_t::J_INTEGER_CONSTANT)
    {
      return token;
    }
  }

  // PARSE FOR JACK IDENTIFIER OR KEYWORD
  if (valid_identifier_char(ch))
  {
    token = get_jack_keyword_or_identifier_token(ch);

    if ((token.value_enum == TokenValue_t::J_CLASS) ||
        (token.value_enum == TokenValue_t::J_CONSTRUCTOR) ||
        (token.value_enum == TokenValue_t::J_FUNCTION) ||
        (token.value_enum == TokenValue_t::J_METHOD) ||
        (token.value_enum == TokenValue_t::J_FIELD) ||
        (token.value_enum == TokenValue_t::J_STATIC) ||
        (token.value_enum == TokenValue_t::J_VAR) ||
        (token.value_enum == TokenValue_t::J_INT) ||
        (token.value_enum == TokenValue_t::J_CHAR) ||
        (token.value_enum == TokenValue_t::J_BOOLEAN) ||
        (token.value_enum == TokenValue_t::J_VOID) ||
        (token.value_enum == TokenValue_t::J_TRUE) ||
        (token.value_enum == TokenValue_t::J_FALSE) ||
        (token.value_enum == TokenValue_t::J_NULL) ||
        (token.value_enum == TokenValue_t::J_THIS) ||
        (token.value_enum == TokenValue_t::J_LET) ||
        (token.value_enum == TokenValue_t::J_DO) ||
        (token.value_enum == TokenValue_t::J_IF) ||
        (token.value_enum == TokenValue_t::J_ELSE) ||
        (token.value_enum == TokenValue_t::J_WHILE) ||
        (token.value_enum == TokenValue_t::J_RETURN) ||

        (token.value_enum == TokenValue_t::J_IDENTIFIER))
    {
      return token;
    }
  }

  // PARSE FOR JACK SYMBOL
  token = get_symbol_token(ch);

  return token;
}

JackToken JackTokenizer::get_line_comment_token(char ch)
{
  assert((ch == '/') && (reader.peek() == '/'));

  auto valid_lcomment_char = [](char c) {
    return (c != 0x0a) && (c != 0x0d) && (c != 0x00);
  };

  std::stringstream s;
  s << ch;

  for (reader.peek(); valid_lcomment_char(reader.peek()); reader.peek())
  {
    ch = reader.read();
    s << ch;
  }

  return JackToken(TokenValue_t::J_COMMENT, s.str(),
                   reader.get_current_line_number());
}

JackToken JackTokenizer::get_block_comment_token(char ch)
{
  JackToken token;

  assert((ch == '/') && (reader.peek() == '*'));

  auto valid_inner_bcomment_char = [&](char c) {
    return (!((c == '*') && (reader.peek() == '/')) && (c != '\0'));
  };

  std::stringstream s;

  s << ch;
  s << reader.read();

  for (ch = reader.read(); valid_inner_bcomment_char(ch); ch = reader.read())
  {
    s << ch;
  }

  if ((ch == '*') && (reader.peek() == '/'))
  {
    s << ch;
    s << reader.read();

    return JackToken(TokenValue_t::J_COMMENT, s.str(),
                     reader.get_current_line_number());
  }

  // an unterminated block can suck up the entire file and be difficult
  // to identify.
  return JackToken(TokenValue_t::J_UNDEFINED, s.str(),
                   reader.get_current_line_number());
}

JackToken JackTokenizer::get_string_token(char ch)
{
  // The Jack spec fails to sufficiently justify why a Jack string is made up
  // of all Unicode characters minus newline and double quote.  My
  // implementation will restrict this to a more convenient ASCII minus
  // newline, double quote, and null.

  auto valid_string_char = [&](char c) {
    if ((c == '\\') && (reader.peek2() == '"'))
      return true;
    if ((c == '\\') && (reader.peek2() == '\\'))
      return true;
    return (c != '\n') && (c != '\0') && (c != '"');
  };

  assert(ch == '"');

  std::stringstream s;

  for (reader.peek(); valid_string_char(reader.peek()); reader.peek())
  {
    ch = reader.read();
    if ((ch == '\\') && (reader.peek() == '"'))
    {
      ch = reader.read();
    }
    if ((ch == '\\') && (reader.peek() == '\\'))
    {
      ch = reader.read();
    }
    s << ch;
  }

  // In a correctly formed string, the next character should be a double quote.
  // If we don't have one, this string is improperly terminated and
  // tokenization is unable to continue.

  if ('"' == reader.peek())
  {
    reader.read();

    return JackToken(TokenValue_t::J_STRING_CONSTANT, s.str(),
                     reader.get_current_line_number());
  }

  return JackToken(TokenValue_t::J_UNDEFINED, s.str(),
                   reader.get_current_line_number());
}

JackToken JackTokenizer::get_integer_token(char ch)
{
  auto valid_integer_char = [](char c) { return (c >= '0') && (c <= '9'); };

  assert(valid_integer_char(ch));

  // At this point, we are certain to have an integer token of unknown length.
  // Text such as '1x', '1class', '1+', and '1 ' while not legal Jack will be
  // thrown out (or accepted) by the parser.

  std::stringstream s;
  s << ch;

  for (reader.peek(); valid_integer_char(reader.peek()); reader.peek())
  {
    ch = reader.read();
    s << ch;
  }

  return JackToken(TokenValue_t::J_INTEGER_CONSTANT, s.str(),
                   reader.get_current_line_number());
}

JackToken JackTokenizer::get_jack_keyword_or_identifier_token(char ch)
{
  // Since keywords can be thought of as a subset of identifiers, keywords
  // will be checked for after identifying any IDENTIFIER U KEYWORD strings.

  assert(valid_identifier_char(ch));

  std::stringstream s;
  s << ch;

  for (reader.peek(); valid_identifier_char(reader.peek()); reader.peek())
  {
    ch = reader.read();
    s << ch;
  }

  //
  // Now we need to determine if we have, in fact, a keyword.
  //

  const std::vector<TokenDescr_t> ExpectedKeywords {
      // clang-format off
      {"do",          TokenValue_t::J_DO, "do"},
      {"if",          TokenValue_t::J_IF, "if"},
      {"int",         TokenValue_t::J_INT, "integer"},
      {"let",         TokenValue_t::J_LET, "let"},
      {"var",         TokenValue_t::J_VAR, "var"},
      {"char",        TokenValue_t::J_CHAR, "character"},
      {"else",        TokenValue_t::J_ELSE, "else"},
      {"null",        TokenValue_t::J_NULL, "null"},
      {"this",        TokenValue_t::J_THIS, "this"},
      {"true",        TokenValue_t::J_TRUE, "true"},
      {"void",        TokenValue_t::J_VOID, "void"},
      {"class",       TokenValue_t::J_CLASS, "class"},
      {"false",       TokenValue_t::J_FALSE, "false"},
      {"field",       TokenValue_t::J_FIELD, "field"},
      {"while",       TokenValue_t::J_WHILE, "while"},
      {"method",      TokenValue_t::J_METHOD, "method"},
      {"return",      TokenValue_t::J_RETURN, "return"},
      {"static",      TokenValue_t::J_STATIC, "static"},
      {"boolean",     TokenValue_t::J_BOOLEAN, "boolean"},
      {"function",    TokenValue_t::J_FUNCTION, "function"},
      {"constructor", TokenValue_t::J_CONSTRUCTOR, "constructor"},
      // clang-format on
  };

  // Return a keyword if that's what it is
  for (const auto& kw : ExpectedKeywords)
  {
    if (s.str() == kw.expected_str)
    {
      return JackToken(kw.token_value, kw.dbg_str,
                       reader.get_current_line_number());
    }
  }

  // Otherwise, its simply an identifier
  return JackToken(TokenValue_t::J_IDENTIFIER, s.str(),
                   reader.get_current_line_number());
}

JackToken JackTokenizer::get_symbol_token(char ch)
{
  JackToken token;

  const std::vector<TokenDescr_t> ExpectedSymbol {
      // clang-format off
      {"{", TokenValue_t::J_LEFT_BRACE, "<left_brace>"},
      {"}", TokenValue_t::J_RIGHT_BRACE, "<right_brace>"},
      {"(", TokenValue_t::J_LEFT_PARENTHESIS, "<left_parenthesis>"},
      {")", TokenValue_t::J_RIGHT_PARENTHESIS, "<right_parenthesis>"},
      {"[", TokenValue_t::J_LEFT_BRACKET, "<left_bracket>"},
      {"]", TokenValue_t::J_RIGHT_BRACKET, "<right_bracket>"},
      {".", TokenValue_t::J_PERIOD, "<period>"},
      {",", TokenValue_t::J_COMMA, "<comma>"},
      {";", TokenValue_t::J_SEMICOLON, "<semicolon>"},
      {"+", TokenValue_t::J_PLUS, "<plus>"},
      {"-", TokenValue_t::J_MINUS, "<minus>"},
      {"*", TokenValue_t::J_ASTERISK, "<asterisk>"},
      {"/", TokenValue_t::J_DIVIDE, "<divide>"},
      {"&", TokenValue_t::J_AMPERSAND, "<ampersand>"},
      {"|", TokenValue_t::J_VBAR, "<vbar>"},
      {"<", TokenValue_t::J_LESS_THAN, "<less_than>"},
      {">", TokenValue_t::J_GREATER_THAN, "<greater_than>"},
      {"=", TokenValue_t::J_EQUAL, "<equal>"},
      {"~", TokenValue_t::J_TILDE, "<tilde>"},
      // clang-format on
  };

  std::string s;
  s = ch;

  for (const auto& sym : ExpectedSymbol)
  {
    if (s == sym.expected_str)
    {
      return JackToken(sym.token_value, sym.dbg_str,
                       reader.get_current_line_number());
    }
  }

  return token;
}

}  // namespace jfcl
