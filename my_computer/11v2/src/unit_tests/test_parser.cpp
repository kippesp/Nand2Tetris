#include "common.h"

SCENARIO("Parse tree simple terms")
{
  SECTION("simple let")
  {
    TextReader R("let i = true;");
    JackTokenizer T(R);

    const JackTokenizer::Tokens_t& tokens = T.parse_tokens();

    recursive_descent::Parser parser(tokens);

    // using Tokens_t = std::vector<JackToken>;
    // std::reference_wrapper<JackTokenizer::Tokens_t> t = *tokens;
    // using Tokens_t = std::vector<JackToken>;
    // std::vector<std::reference_wrapper<JackTokenizer::JackToken>> t;

    // auto parser = recursive_descent::Parser(tokens);
  }
}
