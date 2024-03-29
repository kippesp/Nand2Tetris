#pragma once

#include "util/text_reader.h"

#include <functional>
#include <string>

namespace jfcl {

using TokenValue_t = enum class TokenValue_s {
  J_UNDEFINED,
  J_INTEGER_CONSTANT,
  J_STRING_CONSTANT,
  J_IDENTIFIER,
  J_COMMENT,  // comments stored as internal token
  J_EOF,      // end-of-file and end-of-string

  // Symbols
  J_LEFT_BRACE,         // '{'
  J_RIGHT_BRACE,        // '}'
  J_LEFT_PARENTHESIS,   // '('
  J_RIGHT_PARENTHESIS,  // ')'
  J_LEFT_BRACKET,       // '['
  J_RIGHT_BRACKET,      // ']'
  J_PERIOD,             // '.'
  J_COMMA,              // ','
  J_SEMICOLON,          // ';'
  J_PLUS,               // '+'
  J_MINUS,              // '-'
  J_ASTERISK,           // '*'
  J_DIVIDE,             // '/'
  J_AMPERSAND,          // '&'
  J_VBAR,               // '|'
  J_LESS_THAN,          // '<'
  J_GREATER_THAN,       // '>'
  J_EQUAL,              // '='
  J_TILDE,              // '~'

  // Keywords
  J_CLASS,
  J_CONSTRUCTOR,
  J_FUNCTION,
  J_METHOD,
  J_FIELD,
  J_STATIC,
  J_VAR,
  J_INT,
  J_CHAR,
  J_BOOLEAN,
  J_VOID,
  J_TRUE,
  J_FALSE,
  J_NULL,
  J_THIS,
  J_LET,
  J_DO,
  J_IF,
  J_ELSE,
  J_WHILE,
  J_RETURN,
};

class JackToken;

using JackTokenRef = std::reference_wrapper<JackToken>;
using JackTokenCRef = std::reference_wrapper<const JackToken>;
using Tokens_t = std::vector<JackTokenCRef>;

class JackToken {
public:
  JackToken() = default;
  JackToken(TokenValue_t v, std::string s, int n)
      : value_enum(v), value_str(s), line_number(n)
  {
  }

  friend std::ostream& operator<<(std::ostream& os, JackTokenCRef rhs);

  bool operator==(const JackToken&) const;
  bool operator!=(const JackToken&) const;

  // Convert to text output
  static std::string to_string(TokenValue_t);
  std::string to_s_expression(bool show_line_numbers = false) const;

  TokenValue_t value_enum {TokenValue_t::J_UNDEFINED};
  std::string value_str {"UNDEFINED"};  // for integer, string, comments tokens
  int line_number;                      // don't write big programs!
};

}  // namespace jfcl
