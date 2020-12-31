#include "jack_lexical_elements.h"

#include <cassert>
#include <iostream>
#include <locale>
#include <sstream>
#include <string>

using namespace std;

ostream& operator<<(ostream& os, const JackToken& rhs)
{
  os << "token_name: ";

  os << JackToken::to_string(rhs.type) << endl;
  os << "token_value_str: ";
  os << "<<< " << rhs.value_str << " >>>" << endl;

  return os;
}

string JackToken::to_string(TokenType_t t)
{
  switch (t)
  {
    case TokenType_t::J_UNDEFINED:
      return "UNDEFINED";
    case TokenType_t::J_INTERNAL:
      return "INTERNAL";
    case TokenType_t::J_KEYWORD:
      return "KEYWORD";
    case TokenType_t::J_SYMBOL:
      return "SYMBOL";
    case TokenType_t::J_INTEGER_CONSTANT:
      return "INTEGER_CONSTANT";
    case TokenType_t::J_STRING_CONSTANT:
      return "STRING_CONSTANT";
    case TokenType_t::J_IDENTIFIER:
      return "IDENTIFIER";
  }
}

string JackToken::to_string(TokenValue_t v)
{
  switch (v)
  {
    // clang-format off
    case TokenValue_t::J_UNDEFINED:         return "J_UNDEFINED";
    case TokenValue_t::J_NON_ENUM:          return "J_NON_ENUM";
    case TokenValue_t::J_COMMENT:           return "J_COMMENT";
    case TokenValue_t::J_EOF:               return "J_EOF";
    case TokenValue_t::J_LEFT_BRACE:        return "J_LEFT_BRACE";
    case TokenValue_t::J_RIGHT_BRACE:       return "J_RIGHT_BRACE";
    case TokenValue_t::J_LEFT_PARENTHESIS:  return "J_LEFT_PARENTHESIS";
    case TokenValue_t::J_RIGHT_PARENTHESIS: return "J_RIGHT_PARENTHESIS";
    case TokenValue_t::J_LEFT_BRACKET:      return "J_LEFT_BRACKET";
    case TokenValue_t::J_RIGHT_BRACKET:     return "J_RIGHT_BRACKET";
    case TokenValue_t::J_PERIOD:            return "J_PERIOD";
    case TokenValue_t::J_COMMA:             return "J_COMMA";
    case TokenValue_t::J_SEMICOLON:         return "J_SEMICOLON";
    case TokenValue_t::J_PLUS:              return "J_PLUS";
    case TokenValue_t::J_MINUS:             return "J_MINUS";
    case TokenValue_t::J_ASTERISK:          return "J_ASTERISK";
    case TokenValue_t::J_DIVIDE:            return "J_DIVIDE";
    case TokenValue_t::J_AMPERSAND:         return "J_AMPERSAND";
    case TokenValue_t::J_VBAR:              return "J_VBAR";
    case TokenValue_t::J_LESS_THAN:         return "J_LESS_THAN";
    case TokenValue_t::J_GREATER_THAN:      return "J_GREATER_THAN";
    case TokenValue_t::J_EQUAL:             return "J_EQUAL";
    case TokenValue_t::J_TILDE:             return "J_TILDE";
    case TokenValue_t::J_CLASS:             return "J_CLASS";
    case TokenValue_t::J_CONSTRUCTOR:       return "J_CONSTRUCTOR";
    case TokenValue_t::J_FUNCTION:          return "J_FUNCTION";
    case TokenValue_t::J_METHOD:            return "J_METHOD";
    case TokenValue_t::J_FIELD:             return "J_FIELD";
    case TokenValue_t::J_STATIC:            return "J_STATIC";
    case TokenValue_t::J_VAR:               return "J_VAR";
    case TokenValue_t::J_INT:               return "J_INT";
    case TokenValue_t::J_CHAR:              return "J_CHAR";
    case TokenValue_t::J_BOOLEAN:           return "J_BOOLEAN";
    case TokenValue_t::J_VOID:              return "J_VOID";
    case TokenValue_t::J_TRUE:              return "J_TRUE";
    case TokenValue_t::J_FALSE:             return "J_FALSE";
    case TokenValue_t::J_NULL:              return "J_NULL";
    case TokenValue_t::J_THIS:              return "J_THIS";
    case TokenValue_t::J_LET:               return "J_LET";
    case TokenValue_t::J_DO:                return "J_DO";
    case TokenValue_t::J_IF:                return "J_IF";
    case TokenValue_t::J_ELSE:              return "J_ELSE";
    case TokenValue_t::J_WHILE:             return "J_WHILE";
    case TokenValue_t::J_RETURN:            return "J_RETURN";
      // clang-format on
  }
}

string JackToken::to_s_expression()
{
  stringstream ss;

  ss << "(" << to_string(type);
  ss << " " << to_string(value_enum);
  ss << " " << value_str;
  ss << ")";

  return ss.str();
}
