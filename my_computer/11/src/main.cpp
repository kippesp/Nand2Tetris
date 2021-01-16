#include <string.h>

#include <iostream>

#include "cli_args.h"
#include "tokenizer/file_io.h"
#include "tokenizer/jack_tokenizer.h"
#include "parser/parse_tree.h"

using namespace std;

int inner_main(const CliArgs& cliargs)
{
  for (auto& f : cliargs.inputlist())
  {
    JackInputFile inputfile(f);
    if (inputfile.open())
    {
      cout << "Failed to open input file, " << f << endl;
      return -1;
    }
    JackTokenizer tokenizer(inputfile);
    string base_filename = f.substr(0, f.length() - 5);
    string output_filename = base_filename + ".sexpr";

    auto tokens = tokenizer.parse_tokens();

    if (cliargs.halt_after_tokenizer)
    {
      for (auto& token : *tokens)
        cout << token << endl;
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

    // filter out comment tokens
    auto filtered_tokens = make_unique<vector<JackToken>>();

    for (auto& token : *tokens)
    {
      if (token.value_enum != TokenValue_t::J_COMMENT)
        filtered_tokens->push_back(token);
    }

    ParseTree T(filtered_tokens, base_filename);

    auto parsetree_node = T.parse_class();

    if (cliargs.halt_after_parse_tree_s_expression)
    {
      stringstream ss;
      ss << *parsetree_node;
      cout << T.pprint(ss.str()) << endl;
      return 0;
    }

    ofstream ofile(output_filename);

    if (!ofile)
    {
      cout << "Failed to open output file, " << output_filename << endl;
      return -1;
    }

    stringstream ss;
    ss << *parsetree_node;
    ofile << T.pprint(ss.str()) << endl;
    ofile.close();
  }

  return 0;
}

int main(int argc, const char* argv[])
{
  if (argc == 1)
  {
    cout << "jfcl: missing input" << endl;
    cout << "Try `jfcl -h' for more options." << endl;
    return 1;
  }

  if ((strcmp(argv[1], "--help") == 0) || (strcmp(argv[1], "-h") == 0))
  {
    CliArgs::show_help();
    return 1;
  }

  CliArgs cliargs(argc, argv);

  if ((cliargs.inputlist().size() > 1) &&
      ((cliargs.halt_after_tokenizer ||
        cliargs.halt_after_tokenizer_s_expression)))
  {
    cout << "Output with multiple files not supported." << endl;
    return 1;
  }

  int rvalue;

  try
  {
    rvalue = inner_main(cliargs);
  } catch (ParseException& e)
  {
    cout << "Parse error: " << e.what() << endl;
    cout << e.token.to_s_expression() << endl;
    return 1;
  }

  return rvalue;
}
