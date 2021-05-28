#include "cli_args.h"

#include <stdlib.h>
#include <string.h>

#ifdef LEGACY_CLIARGS
#include <dirent.h>
#include <sys/stat.h>
#else
#include <filesystem>
#endif

#include <cassert>
#include <iomanip>
#include <iostream>
#include <string>

enum class RunMode { unknown, file, directory };

CliArgs::CliArgs(int argc, const char* argv[])
{
  assert(argc > 1);

  int i = 1;

  while (i < argc)
  {
    if (argv[i][0] != '-')
      break;

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
  }

  bool isDirectory = false;

#ifndef LEGACY_CLIARGS
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
    for (auto& entry : std::filesystem::directory_iterator(filearg))
      if (auto filename = entry.path(); filename.extension() == ".jack")
        if (std::filesystem::is_regular_file(filename))
          filelist.push_back(filename.string());
  }
#else
  // Non-Windoz systems with older c++ support can use this implementation.
  struct stat argStat;

  int rvalue = stat(argv[i], &argStat);

  if (rvalue == 0)
  {
    if (argStat.st_mode & S_IFDIR)
    {
      isDirectory = true;
    }
    else if (argStat.st_mode & S_IFREG)
    {
      isDirectory = false;
    }
    else
    {
      std::cerr << "Not a file or directory." << std::endl;
      exit(-1);
    }
  }
  else
  {
    std::cerr << "File or directory not found." << std::endl;
    exit(-1);
  }

  if (!isDirectory)
  {
    std::string filename(argv[i]);

    std::string filenameStem = filename.substr(0, filename.length() - 5);

    if ((filename.length() <= 5) ||
        (filename.substr(filename.length() - 5) != ".jack"))
    {
      std::cerr << "Input filename required to end with .jack extension."
                << std::endl;
      exit(-1);
    }

    filelist.push_back(filename);
  }
  else
  {
    if (DIR* dir = opendir(argv[i]))
    {
      while (true)
      {
        struct dirent* dirEntry;
        dirEntry = readdir(dir);

        if (dirEntry == nullptr)
        {
          if (errno != 0)
          {
            std::cerr << "Unhandled error while reading directory."
                      << std::endl;
            closedir(dir);
            exit(-1);
          }

          break;
        }

        if (strlen(dirEntry->d_name) <= 5)
          continue;

        std::string fileEntry(dirEntry->d_name);

        if (fileEntry.substr(fileEntry.length() - 5) != ".jack")
          continue;

        std::string filePath = std::string(argv[i]) + "/" + dirEntry->d_name;

        rvalue = stat(filePath.c_str(), &argStat);
        bool isFile = false;

        if (rvalue == 0)
        {
          if (argStat.st_mode & S_IFDIR)
          {
            std::cerr << "Directory contains an unsupported directory name."
                      << std::endl;
            closedir(dir);
            exit(-1);
          }
          else if (argStat.st_mode & S_IFREG)
          {
            isFile = true;
          }
          else
          {
            std::cerr << "Not a file or directory." << std::endl;
            closedir(dir);
            exit(-1);
          }
        }
        else
        {
          std::cerr << "Directory contains an unsupported file." << std::endl;
          std::cerr << strerror(errno) << std::endl;
          closedir(dir);
          exit(-1);
        }

        if (!isFile)
        {
          std::cerr << "Directory contains an unsupported filetype."
                    << std::endl;
          closedir(dir);
          exit(-1);
        }

        filelist.push_back(filePath);
      }

      closedir(dir);
    }
    else
    {
      std::cerr << "Unable to read files in directory." << std::endl;
      closedir(dir);
      exit(-1);
    }
  }
#endif
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
  std::cout << "  jfcl DIRECTORY|FILENAME.jack" << std::endl;
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

  std::cout << std::endl;
}
