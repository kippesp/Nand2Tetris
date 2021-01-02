#include <string.h>

#include <iostream>

#include "cli_args.h"
#include "tokenizer/file_io.h"
#include "tokenizer/jack_tokenizer.h"
#include "parser/parse_tree.h"

using namespace std;

int main(int argc, const char* argv[])
{
  (void)argc;
  (void)argv;
  if (argc == 1)
  {
    cout << "jfcl: missing input" << endl;
    cout << "Try `jfcl -h' for more options." << endl;
    exit(1);
  }

  if ((strcmp(argv[1], "--help") == 0) || (strcmp(argv[1], "-h") == 0))
  {
    CliArgs::show_help();
    exit(1);
  }

  CliArgs cliargs(argc, argv);

  if ((cliargs.inputlist().size() > 1) &&
      ((cliargs.halt_after_tokenizer ||
        cliargs.halt_after_tokenizer_s_expression)))
  {
    cout << "Tokenizer output with multiple files not supported." << endl;
    return 1;
  }

  for (auto& f : cliargs.inputlist())
  {
    JackInputFile inputfile(f);
    inputfile.open();
    JackTokenizer tokenizer(inputfile);

    auto tokens = tokenizer.parse_tokens();

    if (cliargs.halt_after_tokenizer)
    {
      for (auto& token : *tokens)
      {
        cout << token << endl;
      }

      return 0;
    }

    if (cliargs.halt_after_tokenizer_s_expression)
    {
      cout << "(TOKENS" << endl;

      for (auto& token : *tokens)
      {
        if (token.type == TokenType_t::J_INTERNAL) continue;

        cout << token.to_s_expression() << endl;
      }

      cout << ")" << endl;

      return 0;
    }

    ParseTree T(ParseTreeNodeType_t::P_UNDEFINED, tokens);
    auto parsetree_node = T.parse_class();

    stringstream ss;
    ss << *parsetree_node;

    cout << ss.str() << endl;
  }

  return 0;
}
