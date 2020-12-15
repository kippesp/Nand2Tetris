#include <string.h>

#include <iostream>

#include "cli_args.h"
#include "tokenizer/file_io.h"
#include "tokenizer/jack_tokenizer.h"

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

  if ((cliargs.inputlist().size() > 1) && (cliargs.halt_after_tokenizer))
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
  }

  return 0;
}
