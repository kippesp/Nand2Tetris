#include <string>

typedef enum class TokenType_s {
  J_UNDEFINED,
  J_INTERNAL,

  J_KEYWORD,
  J_SYMBOL,
  J_INTEGER_CONSTANT,
  J_STRING_CONSTANT,
  J_IDENTIFIER,
} TokenType_t;

typedef enum class TokenValue_s {
  J_UNDEFINED,
  J_NON_ENUM,
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
} TokenValue_t;

struct JackToken {
  JackToken() {}
  JackToken(TokenType_t t, TokenValue_t v, std::string s)
      : type(t), value_enum(v), value_str(s)
  {
  }

  friend std::ostream& operator<<(std::ostream& os, const JackToken& rhs);

  TokenType_t type{TokenType_t::J_UNDEFINED};
  TokenValue_t value_enum{TokenValue_t::J_UNDEFINED};
  std::string value_str{"UNDEFINED"};  // for integer, string, comments tokens

  // Convert to text output
  static std::string to_string(TokenType_t);
  static std::string to_string(TokenValue_t);
  std::string to_s_expression() const;
};
