#include "cli_args.h"

#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <cassert>
#include <iomanip>
#include <iostream>
#include <string>

using std::cout;
using std::left;
using std::setw;

enum class RunMode { unknown, file, directory };

CliArgs::CliArgs(int argc, const char* argv[])
{
  assert(argc > 1);

  int i = 1;

  while (i < argc)
  {
    if (argv[i][0] != '-') break;

    // -t - stop after tokenizer and output its values
    if ((argv[i][0] == '-') && (argv[i][1] == 't') && (argv[i][2] == '\0'))
    {
      halt_after_tokenizer = true;
      i++;
      continue;
    }

    // -T - stop after tokenizer in xml and output its values
    if ((argv[i][0] == '-') && (argv[i][1] == 'T') && (argv[i][2] == '\0'))
    {
      halt_after_tokenizer_xml = true;
      i++;
      continue;
    }
  }

  struct stat argStat;
  bool isDirectory = false;

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
    DIR* dir = opendir(argv[i]);

    if (dir == NULL)
    {
      std::cerr << "Unable to read files in directory." << std::endl;
      exit(-1);
    }

    while (true)
    {
      struct dirent* dirEntry;

      dirEntry = readdir(dir);

      if (dirEntry == NULL)
      {
        if (errno != 0)
        {
          std::cerr << "Unhandled error while reading directory." << std::endl;
          exit(-1);
        }

        break;
      }

      if (strlen(dirEntry->d_name) <= 3) continue;

      std::string fileEntry(dirEntry->d_name);

      if (fileEntry.substr(fileEntry.length() - 5) != ".jack") continue;

      std::string filePath = std::string(argv[i]) + "/" + dirEntry->d_name;

      rvalue = stat(filePath.c_str(), &argStat);
      bool isFile = false;

      if (rvalue == 0)
      {
        if (argStat.st_mode & S_IFDIR)
        {
          std::cerr << "Directory contains an unsupported directory name."
                    << std::endl;
          exit(-1);
        }
        else if (argStat.st_mode & S_IFREG)
        {
          isFile = true;
        }
        else
        {
          std::cerr << "Not a file or directory." << std::endl;
          exit(-1);
        }
      }
      else
      {
        std::cerr << "Directory contains an unsupported file." << std::endl;
        std::cerr << strerror(errno) << std::endl;
        exit(-1);
      }

      if (!isFile)
      {
        std::cerr << "Directory contains an unsupported filetype." << std::endl;
        exit(-1);
      }

      filelist.push_back(fileEntry);
    }
  }
}

const std::list<const std::string>& CliArgs::inputlist()
{
  return filelist;
}

void CliArgs::show_usage()
{
  std::cout << "SYNOPSIS:\n\n";
  std::cout << "  jfcl -h" << std::endl;
  std::cout << "  jfcl [-t] FILENAME.jack" << std::endl;
  std::cout << "  jfcl DIRECTORY|FILENAME.jack" << std::endl;
}

void CliArgs::show_help()
{
  std::cout << "jfcl - Jack Front End Compiler\n\n";
  show_usage();

  std::cout << "\nOPTIONS:\n";

  std::cout << "\n  ";
  std::cout << setw(24) << left << "-h";
  std::cout << "Display available options";

  std::cout << "\n  ";
  std::cout << setw(24) << left << "-t";
  std::cout << "Display tokenizer output and halt (includes internal tokens)";

  std::cout << "\n  ";
  std::cout << setw(24) << left << "-T";
  std::cout << "Display tokenizer output in xml and halt";

  std::cout << std::endl;

#if 0
    std::cout << "\n  ";
    std::cout << setw(24) << left << "-h";
    std::cout << "Display available options\n";
#endif
}
