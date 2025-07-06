#include "cli_args.h"

#include <cassert>
#include <filesystem>
#include <iostream>

namespace jfcl {

enum class RunMode { unknown, file, directory };

CliArgs::CliArgs(int argc, const char* argv[])
{
  assert(argc > 1);

  int i = 1;

  while (i < argc)
  {
    if (argv[i][0] != '-')
    {
      break;
    }

    // -t - stop after tokenizer and output its values
    if ((argv[i][0] == '-') && (argv[i][1] == 't') && (argv[i][2] == '\0'))
    {
      halt_after_tokenizer = true;
      i++;
      continue;
    }

    // -p - stop after parser and output the parse tree
    if ((argv[i][0] == '-') && (argv[i][1] == 'p') && (argv[i][2] == '\0'))
    {
      halt_after_parse_tree_s_expression = true;
      i++;
      continue;
    }

    // -w - stop after first lowering to VM output to console
    if ((argv[i][0] == '-') && (argv[i][1] == 'w') && (argv[i][2] == '\0'))
    {
      halt_after_vmwriter = true;
      i++;
      continue;
    }

    // -r - enable new operator precedence parsing
    //      Use this to enable full operator precedence (experimental)
    if ((argv[i][0] == '-') && (argv[i][1] == 'r') && (argv[i][2] == '\0'))
    {
      enable_precedence_parsing = true;
      i++;
      continue;
    }

    // -l - left justify VM output (remove indentation)
    //      Use this to match reference compiler formatting
    if ((argv[i][0] == '-') && (argv[i][1] == 'l') && (argv[i][2] == '\0'))
    {
      left_justify_vm_output = true;
      i++;
      continue;
    }
  }

  bool isDirectory = false;

  // Cleaner implementation with modern std::filesystem support (VS2019,
  // g++-8, llvm-7)
  auto filearg = std::filesystem::path(argv[i]);

  if (std::filesystem::is_directory(filearg))
  {
    isDirectory = true;
  }
  else if (std::filesystem::is_regular_file(filearg))
  {
    isDirectory = false;
  }
  else
  {
    std::cerr << "Not a file or directory." << std::endl;
    exit(-1);
  }

  if (!isDirectory)
  {
    if (filearg.extension() != ".jack")
    {
      std::cerr << "Input filename required to end with .jack extension."
                << std::endl;
      exit(-1);
    }

    filelist.push_back(filearg.string());
  }
  else
  {
    for (const auto& entry : std::filesystem::directory_iterator(filearg))
    {
      if (auto filename = entry.path(); filename.extension() == ".jack")
      {
        if (std::filesystem::is_regular_file(filename))
        {
          filelist.push_back(filename.string());
        }
      }
    }
  }
}

const CliArgs::filelist_t& CliArgs::inputlist() const
{
  return filelist;
}

void CliArgs::show_usage()
{
  std::cout << "Compile .jack file to .vm\n\n";
  std::cout << "SYNOPSIS:\n\n";
  std::cout << "  jfcl -h" << std::endl;
  std::cout << "  jfcl [-t|-p|-w] FILENAME.jack" << std::endl;
  std::cout << "  jfcl [-r|-l] DIRECTORY|FILENAME.jack" << std::endl;
}

void CliArgs::show_help()
{
  std::cout << "jfcl - Jack Front End Compiler\n\n";
  show_usage();

  std::cout << "\nOPTIONS:\n";

  std::cout << "\n  ";
  std::cout << std::setw(24) << std::left << "-h";
  std::cout << "Display available options";

  std::cout << "\n  ";
  std::cout << std::setw(24) << std::left << "-t";
  std::cout
      << "Display tokenizer output in S expression w/o internals and halt";

  std::cout << "\n  ";
  std::cout << std::setw(24) << std::left << "-p";
  std::cout << "Display parse tree output and halt";

  std::cout << "\n  ";
  std::cout << std::setw(24) << std::left << "-w";
  std::cout << "Display VM Writer output and halt";

  std::cout << "\n  ";
  std::cout << std::setw(24) << std::left << "-r";
  std::cout << "Enable new operator precedence parsing";

  std::cout << "\n  ";
  std::cout << std::setw(24) << std::left << "-l";
  std::cout << "Left justify VM output (no indentation)";

  std::cout << std::endl;
}

}  // namespace jfcl
