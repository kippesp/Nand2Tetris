#include "jack_token.h"

#include <cassert>
#include <iostream>
#include <locale>
#include <sstream>
#include <string>

std::ostream& operator<<(std::ostream& os, JackTokenCRef rhs)
{
  os << "token_name: ";

  os << JackToken::to_string(rhs.get().type) << std::endl;
  os << "token_value_str: ";
  os << "<<< " << rhs.get().value_str << " >>>" << std::endl;

  return os;
}

std::string JackToken::to_string(TokenType_t t)
{
  switch (t)
  {
    // clang-format off
    case TokenType_t::J_UNDEFINED:          return "UNDEFINED";
    case TokenType_t::J_INTERNAL:           return "INTERNAL";
    case TokenType_t::J_KEYWORD:            return "KEYWORD";
    case TokenType_t::J_SYMBOL:             return "SYMBOL";
    case TokenType_t::J_INTEGER_CONSTANT:   return "INTEGER_CONSTANT";
    case TokenType_t::J_STRING_CONSTANT:    return "STRING_CONSTANT";
    case TokenType_t::J_IDENTIFIER:         return "IDENTIFIER";
      // clang-format on
  }

  assert(0 && "fallthrough");
}

std::string JackToken::to_string(TokenValue_t v)
{
  switch (v)
  {
    // clang-format off
    case TokenValue_t::J_UNDEFINED:         return "UNDEFINED";
    case TokenValue_t::J_NON_ENUM:          return "NON_ENUM";
    case TokenValue_t::J_COMMENT:           return "COMMENT";
    case TokenValue_t::J_EOF:               return "EOF";
    case TokenValue_t::J_LEFT_BRACE:        return "LEFT_BRACE";
    case TokenValue_t::J_RIGHT_BRACE:       return "RIGHT_BRACE";
    case TokenValue_t::J_LEFT_PARENTHESIS:  return "LEFT_PARENTHESIS";
    case TokenValue_t::J_RIGHT_PARENTHESIS: return "RIGHT_PARENTHESIS";
    case TokenValue_t::J_LEFT_BRACKET:      return "LEFT_BRACKET";
    case TokenValue_t::J_RIGHT_BRACKET:     return "RIGHT_BRACKET";
    case TokenValue_t::J_PERIOD:            return "PERIOD";
    case TokenValue_t::J_COMMA:             return "COMMA";
    case TokenValue_t::J_SEMICOLON:         return "SEMICOLON";
    case TokenValue_t::J_PLUS:              return "PLUS";
    case TokenValue_t::J_MINUS:             return "MINUS";
    case TokenValue_t::J_ASTERISK:          return "ASTERISK";
    case TokenValue_t::J_DIVIDE:            return "DIVIDE";
    case TokenValue_t::J_AMPERSAND:         return "AMPERSAND";
    case TokenValue_t::J_VBAR:              return "VBAR";
    case TokenValue_t::J_LESS_THAN:         return "LESS_THAN";
    case TokenValue_t::J_GREATER_THAN:      return "GREATER_THAN";
    case TokenValue_t::J_EQUAL:             return "EQUAL";
    case TokenValue_t::J_TILDE:             return "TILDE";
    case TokenValue_t::J_CLASS:             return "CLASS";
    case TokenValue_t::J_CONSTRUCTOR:       return "CONSTRUCTOR";
    case TokenValue_t::J_FUNCTION:          return "FUNCTION";
    case TokenValue_t::J_METHOD:            return "METHOD";
    case TokenValue_t::J_FIELD:             return "FIELD";
    case TokenValue_t::J_STATIC:            return "STATIC";
    case TokenValue_t::J_VAR:               return "VAR";
    case TokenValue_t::J_INT:               return "INT";
    case TokenValue_t::J_CHAR:              return "CHAR";
    case TokenValue_t::J_BOOLEAN:           return "BOOLEAN";
    case TokenValue_t::J_VOID:              return "VOID";
    case TokenValue_t::J_TRUE:              return "TRUE";
    case TokenValue_t::J_FALSE:             return "FALSE";
    case TokenValue_t::J_NULL:              return "NULL";
    case TokenValue_t::J_THIS:              return "THIS";
    case TokenValue_t::J_LET:               return "LET";
    case TokenValue_t::J_DO:                return "DO";
    case TokenValue_t::J_IF:                return "IF";
    case TokenValue_t::J_ELSE:              return "ELSE";
    case TokenValue_t::J_WHILE:             return "WHILE";
    case TokenValue_t::J_RETURN:            return "RETURN";
      // clang-format on
  }

  assert(0 && "fallthrough");
}

std::string JackToken::to_s_expression() const
{
  std::stringstream ss;

  ss << "(" << to_string(type);
  ss << " " << to_string(value_enum);
  ss << " " << value_str;
  ss << ")";

  return ss.str();
}
