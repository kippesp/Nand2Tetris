#include <iostream>

#include "common.h"
#include "jack_sources.h"

#if 0
SCENARIO("Expressions")
{
  SECTION("simple identifier")
  {
    TextReader R("myvar");
    JackTokenizer T(R);

    auto tokens = T.parse_tokens();

    pratt::Parser parser(tokens);
    const auto& root = parser.parse_expression();
#if 0
    std::string as_str = root.get().as_s_expression();

    Expected_t expected = {
        ""  // clang-format sorcery
        "(EXPRESSION",
        "  (INTEGER_CONSTANT string_value:1))"};

    std::string expected_str = expected_string(expected);
    REQUIRE(as_str == expected_str);
#endif
  }
}
#endif
