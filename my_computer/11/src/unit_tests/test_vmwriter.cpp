#include <string.h>

#include "catch.hpp"
#include "jack_sources.h"
#include "mock_reader.h"
#include "parser/parse_tree.h"
#include "vmwriter/vmwriter.h"

using namespace std;

SCENARIO("VMWriter Basics")
{
  test::MockReader R;

  SECTION("Simple tree dfs traversal")
  {
    strcpy(R.buffer, JACK_SEVEN_SRC);
    JackTokenizer Tokenizer(R);
    auto tokens = Tokenizer.parse_tokens();

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_class();
    REQUIRE(parsetree_node);

    VmWriter VM(parsetree_node);
    stringstream ss;

    while (auto node = VM.visit())
      ss << ParseTreeNode::to_string(node->type) << " ";

    // verify tree structure using strings
    REQUIRE(
        ss.str() ==
        "P_CLASS_DECL_BLOCK P_KEYWORD P_CLASS_NAME P_DELIMITER "
        "P_SUBROUTINE_DECL_BLOCK P_SUBROUTINE_TYPE P_RETURN_TYPE "
        "P_SUBROUTINE_NAME P_DELIMITER P_PARAMETER_LIST P_DELIMITER "
        "P_SUBROUTINE_BODY P_DELIMITER P_STATEMENT_LIST P_DO_STATEMENT "
        "P_KEYWORD P_SUBROUTINE_CALL P_CLASS_OR_VAR_NAME P_DELIMITER "
        "P_SUBROUTINE_NAME P_DELIMITER P_EXPRESSION_LIST P_EXPRESSION P_TERM "
        "P_INTEGER_CONSTANT P_OP P_TERM P_DELIMITER P_EXPRESSION P_TERM "
        "P_INTEGER_CONSTANT P_OP P_TERM P_INTEGER_CONSTANT P_DELIMITER "
        "P_DELIMITER P_DELIMITER P_RETURN_STATEMENT P_KEYWORD P_DELIMITER "
        "P_DELIMITER P_DELIMITER ");
  }
}
