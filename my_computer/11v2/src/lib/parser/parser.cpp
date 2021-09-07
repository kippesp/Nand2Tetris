#include "parser.h"
#include <signal.h>

namespace recursive_descent {

Parser::Parser(const JackTokenizer::Tokens_t& tokens)
    : token_iter(tokens.begin()),
      current_token(*token_iter),
      next_token(*(++token_iter))
{
  raise(SIGTRAP);
}

#if 0
parse_class
#endif

#if 0
let i = true;

(TOKENS
(KEYWORD J_LET let)
(IDENTIFIER J_NON_ENUM i)
(SYMBOL J_EQUAL <equal>)
(KEYWORD J_TRUE true)
(SYMBOL J_SEMICOLON <semicolon>)
)

(N_LET_STATEMENT
  (N_SCALAR_VAR
    (N_VARIABLE_NAME i))
  (N_EXPRESSION
    (N_TERM
      (N_KEYWORD_CONSTANT
        (N_TRUE_KEYWORD)))))
#endif

}  // namespace recursive_descent
