typedef enum class JackKeywordName_s {
  J_UNDEFINED,
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
} JackKeywordName_t;

typedef enum class JackSymbolName_s {
  J_UNDEFINED,
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
} JackSymbolName_t;
