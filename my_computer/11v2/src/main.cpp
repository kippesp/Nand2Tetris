#include <string.h>

#include <iostream>

#include "parser/parser.h"
#include "tokenizer/jack_tokenizer.h"
#include "util/cli_args.h"
#include "util/textfile_reader.h"
//#include "vmwriter/vmwriter.h"

static int inner_main(const CliArgs& cliargs)
{
  for (const auto& f : cliargs.inputlist())
  {
    TextFileReader inputfile(f.data());
    JackTokenizer tokenizer(inputfile);
    std::string base_filename = f.substr(0, f.length() - 5);
    std::string output_filename = base_filename + ".vm";

    const auto tokens = tokenizer.parse_tokens();

    if (cliargs.halt_after_tokenizer)
    {
      std::cout << "(TOKENS" << std::endl;

      for (auto& token : tokens)
      {
        if (token.get().value_enum == TokenValue_t::J_COMMENT)
        {
          continue;
        }

        std::cout << token.get().to_s_expression() << std::endl;
      }

      std::cout << ")" << std::endl;
      return 0;
    }

    // filter out comment tokens
    Tokens_t filtered_tokens;

    for (const auto& token : tokens)
    {
      if (token.get().value_enum != TokenValue_t::J_COMMENT)
      {
        filtered_tokens.push_back(token);
      }
    }

    recursive_descent::Parser parser(filtered_tokens);

    const auto& class_ast = parser.parse_class();

    if (cliargs.halt_after_parse_tree_s_expression)
    {
      std::stringstream ss;
      ss << class_ast.get().as_s_expression();
      std::cout << ss.str() << std::endl;
      return 0;
    }

#if 0
    VmWriter VM(parsetree_node);
    VM.lower_class();

    if (cliargs.halt_after_vmwriter)
    {
      cout << VM.lowered_vm.str();
      return 0;
    }

    ofstream ofile(output_filename);

    if (!ofile)
    {
      cout << "Failed to open output file, " << output_filename << endl;
      return -1;
    }

    ofile << VM.lowered_vm.str();
    ofile.close();
#endif
  }

  return 0;
}

int main(int argc, const char* argv[])
{
  if (argc == 1)
  {
    std::cout << "jfcl: missing input" << std::endl;
    std::cout << "Try `jfcl -h' for more options." << std::endl;
    return 1;
  }

  if ((strcmp(argv[1], "--help") == 0) || (strcmp(argv[1], "-h") == 0))
  {
    CliArgs::show_help();
    return 1;
  }

  CliArgs cliargs(argc, argv);

  if ((cliargs.inputlist().size() > 1) &&
      ((cliargs.halt_after_tokenizer || cliargs.halt_after_vmwriter ||
        cliargs.halt_after_parse_tree_s_expression)))
  {
    std::cout << "Output with multiple files not supported." << std::endl;
    return 1;
  }

  int rvalue;

#if 0
  try
  {
#endif
  rvalue = inner_main(cliargs);
#if 0
  } catch (ParseException& e)
  {
    cout << "Parse error: " << e.what() << endl;
    cout << e.token.to_s_expression() << endl;
    return 1;
  }
#endif

  return rvalue;
}
