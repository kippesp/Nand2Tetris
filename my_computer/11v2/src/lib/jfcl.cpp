#include "jfcl.h"

#include "parser/parser.h"
#include "tokenizer/jack_tokenizer.h"
#include "util/cli_args.h"
#include "util/textfile_reader.h"
#include "vmwriter/vmwriter.h"

#include <fstream>
#include <iostream>

using namespace jfcl;

static int jfcl_inner_main(const CliArgs& cliargs)
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

    AstTree ast;
    Parser parser(filtered_tokens, ast);

    if (!cliargs.enable_precedence_parsing)
    {
      parser.set_left_associative();
    }

    std::string class_name;
    const auto& class_ast = parser.parse_class(class_name).get();

    if (cliargs.halt_after_parse_tree_s_expression)
    {
      std::stringstream ss;
      ss << class_ast.as_s_expression(false);
      std::cout << ss.str() << std::endl;
      return 0;
    }

    jfcl::VmWriter VM(parser.get_ast(), cliargs.left_justify_vm_output);
    VM.lower_module();

    if (cliargs.halt_after_vmwriter)
    {
      std::cout << VM.get_lowered_vm();
      return 0;
    }

    std::ofstream ofile(output_filename);

    if (!ofile)
    {
      std::cout << "Failed to open output file, " << output_filename
                << std::endl;
      return -1;
    }

    ofile << VM.get_lowered_vm();
    ofile.close();
  }

  return 0;
}

JFCL_DLLEXPORT int JFCL_APICALL jfcl_main(int argc, const char* argv[])
{
  if (argc <= 1)
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

  try
  {
    rvalue = jfcl_inner_main(cliargs);
  }
  catch (const std::runtime_error& e)
  {
    std::cout << "Error: " << e.what() << std::endl;
    return 1;
  }
  catch (const std::domain_error& e)
  {
    std::cout << "Error: " << e.what() << std::endl;
    return 1;
  }
  catch (...)
  {
    std::cout << "Unhandled exception" << std::endl;
    return 1;
  }

  return rvalue;
}
