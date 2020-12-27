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

string JackToken::to_xml()
{
  locale loc;
  stringstream ts;

  string t = to_string(type);
  for (char c : t)
  {
    ts << tolower(c, loc);
  }

  t = ts.str();

  if (t == "integer_constant")
    t = "integerConstant";
  else if (t == "string_constant")
    t = "stringConstant";

  stringstream s;

  s << "<" << t << "> ";

  if (type == TokenType_t::J_KEYWORD)
  {
    for (char c : value_str)
    {
      s << tolower(c, loc);
    }
  }
  else if (type == TokenType_t::J_SYMBOL)
  {
    int skip = 0;

    for (char c : value_str)
    {
      if (skip == 1)
      {
        if (c == '<')
          s << "&lt;";
        else if (c == '>')
          s << "&gt;";
        else if (c == '&')
          s << "&amp;";
        else if (c == '"')
          s << "&quot;";
        else
          s << c;
        skip++;
      }

      if (c == ' ') skip++;
    }
  }
  else
  {
    s << value_str;
  }

  s << " </" << t << ">";

  return s.str();
}
