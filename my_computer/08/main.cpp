#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <list>
#include <sys/stat.h>

#ifdef CPP17_LATER
# include <filesystem>
#else
# include <dirent.h>
#endif

// TODO: Add metrics.

// R13-R15 GP registers

using namespace std;

const int STATIC_SEGMENT_BASE = 16;

typedef enum {
  C_NONE,
  C_ARITHMETIC,
  C_PUSH,
  C_POP,
  C_LABEL,
  C_GOTO,
  C_IF_GOTO,
  C_FUNCTION,
  C_RETURN,
  C_CALL
} Command_t;

/* Parser - Handles the parsing of a single .vm file     */
/*          Reads VM cmmands, parses them, and provides  */
/*          access to their components.  All white space */
/*          and comments are removed.                    */
class Parser {
  string currentLine;
  list<string> allLinesInFile;
  list<int> allLineNumbers;
  list<string> commandAndArguments; // list: cmd type, arg1, arg2
  int commandLineNumber;

public:

  // Open the input file and get ready to parse it
  Parser(string filenameStem) : commandLineNumber(0)
  {
    ifstream infile;
    string filename = filenameStem + ".vm";

    infile.open(filename, ios::in);

    if (!infile.is_open())
    {
      cerr << "Failed to open input file, " << filename << endl;
      exit(-2);
    }

    int currentLineNumber = 0;

    // Read the entire file into the list `allLinesInFile` to make
    // detection of EOF a little less cumbersome.
    while (!infile.eof())
    {
      string line;

      getline(infile, line);
      currentLineNumber++;

      // trim all leading whitespace
      const auto start = line.find_first_not_of(" \t");

      if (start == string::npos)
        continue;

      line = line.substr(start, string::npos);

      // ignore line if first two characters are '//'
      if (line.substr(0, 2) == "//")
        continue;

      allLinesInFile.push_back(line);
      allLineNumbers.push_back(currentLineNumber);
    }

    infile.close();
  }

  // Are there more commands in the input?
  bool hasMoreCommands()
  {
    return !allLinesInFile.empty();
  }

  // Reads the next command from the input file and makes it the
  // current command.  Should be called only if hasMoreCommands() is true.
  // Initially there is no current command.
  void advance()
  {
    assert(hasMoreCommands());

    auto currentLine = allLinesInFile.front();
    commandLineNumber = allLineNumbers.front();

    transform(currentLine.begin(), currentLine.end(), currentLine.begin(),
        ::tolower);

    // Process and tokenize the line into the
    commandAndArguments.clear();

    // For each space separated word, accumulate the word into `acc`.
    // Once a word is identified, add it to the list `commandAndArguments`
    string acc = "";
    for (const char& c : currentLine)
    {
      if (c == ' ')
      {
        commandAndArguments.push_back(acc);
        acc = "";
      }
      else
      {
        acc = acc + c;
      }
    }

    commandAndArguments.push_back(acc);

    allLinesInFile.pop_front();
    allLineNumbers.pop_front();
  }

  // Returns the type of the current VM command.  C_ARITHMETIC is returned
  // for all the arithmetic commands.
  Command_t commandType()
  {
    string command = commandAndArguments.front();
  
    if (command == "push") return C_PUSH;
    if (command == "pop")  return C_POP;
    if (command == "eq")   return C_ARITHMETIC;
    if (command == "lt")   return C_ARITHMETIC;
    if (command == "gt")   return C_ARITHMETIC;
    if (command == "add")  return C_ARITHMETIC;
    if (command == "sub")  return C_ARITHMETIC;
    if (command == "neg")  return C_ARITHMETIC;
    if (command == "and")  return C_ARITHMETIC;
    if (command == "or")   return C_ARITHMETIC;
    if (command == "not")  return C_ARITHMETIC;

    assert("Unsupported command");

    return C_NONE;
  }

  // Returns the first argument of the current command.  In case of
  // C_ARITHMETIC, the command itself (add, sub, etc.) is returned.
  // Should not be called if the current command is C_RETURN.
  string arg1()
  {
    Command_t cmdType = commandType();

    assert(cmdType != C_RETURN);

    auto itr = commandAndArguments.begin();

    if (cmdType == C_ARITHMETIC)
    {
      string command = *itr;

      return command;
    }

    itr++;

    if ((cmdType == C_PUSH) || (cmdType == C_POP))
    {
      string argument = *itr;

      return argument;
    }

    assert(0);

    return "";
  }

  // Returns the second argument of the current command.  Should be called
  // only if the current command is C_PUSH, C_POP, C_FUNCTION,
  // or C_CALL.
  int args()
  {
    Command_t cmdType = commandType();

    assert((cmdType == C_PUSH) || (cmdType == C_POP) ||
        (cmdType == C_FUNCTION) || (cmdType == C_CALL));

    auto itr = commandAndArguments.begin();
    itr++;
    itr++;

    string argument = *itr;
    int value;

    value = stoi(argument, nullptr);

    return value;
  }

  int lineNumber()
  {
    return commandLineNumber;
  }
};

/* CodeWriter - Translates VM commands into Hack assembly code. */
class CodeWriter {
  ofstream outfile;
  unsigned int branchNumber = 0;
  const string filenameStem;
  const string filename;
  string currentFunction = "anonymous";

  // TODO: add and call to "createLabel"
  string createBranchTag(int counter)
  {
     // TODO: Set as "functionName$label"
     auto i = filenameStem.rfind('/', filenameStem.length());
     string tag;

     if (i != string::npos)
     {
       tag = filenameStem.substr(i+1, filenameStem.length() - i);
     }

    return to_string(counter) + "$" + tag;
  }

public:

  CodeWriter(string filenameStem) : filenameStem(filenameStem),
                                    filename(filenameStem + ".asm")
  {
    outfile.open(filename, ofstream::out);

    if (!outfile.is_open())
    {
      cerr << "Failed to open output file, " << filename << endl;
      exit(-2);
    }

    outfile << "// File: " << filename << endl;
  }

  ~CodeWriter()
  {
    outfile.close();
  }

  // See section 7.2.3 - Memory Access Commands
  void writeArithmetic(int lineNumber, string command)
  {
    outfile << "// " << lineNumber << ": " << command << endl;

    // add/sub - binary
    if ((command == "add") || (command == "sub") ||
        (command == "and") || (command == "or"))
    {
      string operation;

      outfile << "@SP" << endl;
      outfile << "M=M-1" << endl;
      outfile << "A=M" << endl;
      outfile << "D=M" << endl;
      outfile << "A=A-1" << endl;

      if      (command == "add") operation = "M=D+M";
      else if (command == "sub") operation = "M=M-D";
      else if (command == "and") operation = "M=D&M";
      else if (command == "or" ) operation = "M=D|M";

      outfile << operation << endl;
    }

    // eq/gt/lt - binary
    // output true(-1) if condition true
    // output false(0) if not
    else if ((command == "eq") || (command == "lt") || (command == "gt"))
    {
      branchNumber++;

      outfile << "@SP" << endl;   // SP' = SP
      outfile << "M=M-1" << endl; // SP  = SP'-1
      outfile << "A=M" << endl;   // A   = SP'-1    = SP-2
      outfile << "D=M" << endl;   // D   = *[SP'-1] = *[SP'-2]
      outfile << "A=A-1" << endl; // A   = SP'-2
      outfile << "D=M-D" << endl; // D   = *[SP'-2] - *[SP'-1]

      outfile << "@CMP_" << createBranchTag(branchNumber) << endl;

      if (command == "eq")
        outfile << "D;JEQ" << endl;
      else if (command == "lt")
        outfile << "D;JLT" << endl;
      else
        outfile << "D;JGT" << endl;

      outfile << "D=0" << endl;
      outfile << "@JOIN_CMP_" << createBranchTag(branchNumber) << endl;
      outfile << "0;JMP" << endl;
      outfile << "(CMP_" << createBranchTag(branchNumber) << ")" << endl;
      outfile << "D=-1" << endl;
      outfile << "(JOIN_CMP_" << createBranchTag(branchNumber) << ")" << endl;

      outfile << "@SP" << endl;
      outfile << "A=M-1" << endl; // A = SP-1 = SP'-1-1 = SP'-2
      outfile << "M=D" << endl;   // *[SP'-2] = D
    }

    else if ((command == "neg") || (command == "not"))
    {
      outfile << "@SP" << endl;
      outfile << "A=M-1" << endl;
      outfile << "D=M" << endl;
      outfile << ((command == "neg") ? "M=-D" : "M=!D") << endl;
    }

    else
    {
      cerr << command << endl;
      assert(0);
    }
  }

  // See section 7.2.3 - Memory Access Commands
  void writePushPop(int lineNumber, Command_t command, string segment,
      int index)
  {
    if (command == C_PUSH)
    {
      outfile << "// " << lineNumber << ": push " << segment << " " << index << endl;

      if ((segment == "local") || (segment == "argument") ||
          (segment == "this") || (segment == "that"))
      {
        // D <-- *segment_base + index
        if (segment == "local")
        {
          outfile << "@" << "LCL" << endl;
        }
        else if (segment == "argument")
        {
          outfile << "@" << "ARG" << endl;
        }
        else if (segment == "this")
        {
          outfile << "@" << "THIS" << endl;
        }
        else if (segment == "that")
        {
          outfile << "@" << "THAT" << endl;
        }

        outfile << "D=M" << endl;
        outfile << "@" << index << endl;
        outfile << "D=D+A" << endl;
        outfile << "A=D" << endl;
        outfile << "D=M" << endl;

        // push D onto stack
        outfile << "@SP" << endl;
        outfile << "A=M" << endl;
        outfile << "M=D" << endl;
        outfile << "@SP" << endl;
        outfile << "M=M+1" << endl;
      }
      else if (segment == "constant")
      {
        outfile << "@" << index << endl;
        outfile << "D=A" << endl;

        // push D onto stack
        outfile << "@SP" << endl;
        outfile << "A=M" << endl;
        outfile << "M=D" << endl;
        outfile << "@SP" << endl;
        outfile << "M=M+1" << endl;
      }
      else if (segment == "temp")
      {
        assert(index <= 8);

        outfile << "@" << "R" << 5 + index << endl;
        outfile << "D=M" << endl;

        // push D onto stack
        outfile << "@SP" << endl;
        outfile << "A=M" << endl;
        outfile << "M=D" << endl;
        outfile << "@SP" << endl;
        outfile << "M=M+1" << endl;
      }
      else if (segment == "static")
      {
        // Directly compute the absolute address
        int staticAddress = STATIC_SEGMENT_BASE + index;

        outfile << "@" << staticAddress << endl;
        outfile << "D=M" << endl;

        // push D onto stack
        outfile << "@SP" << endl;
        outfile << "A=M" << endl;
        outfile << "M=D" << endl;
        outfile << "@SP" << endl;
        outfile << "M=M+1" << endl;
      }
      else if (segment == "pointer")
      {
        assert(index <= 2);

        string selectedRegister = (index == 0) ? "@THIS" : "@THAT";

        outfile << selectedRegister << endl;

        outfile << "D=M" << endl;

        // push D onto stack
        outfile << "@SP" << endl;
        outfile << "A=M" << endl;
        outfile << "M=D" << endl;
        outfile << "@SP" << endl;
        outfile << "M=M+1" << endl;
      }
      else
      {
        assert(0);
      }
    }
    else if (command == C_POP)
    {
      outfile << "// " << lineNumber << ": pop " << segment << " " << index << endl;

      if ((segment == "local") || (segment == "argument") ||
          (segment == "this") || (segment == "that"))
      {
        // D <-- *segment_base + index
        if (segment == "local")
        {
          outfile << "@" << "LCL" << endl;
        }
        else if (segment == "argument")
        {
          outfile << "@" << "ARG" << endl;
        }
        else if (segment == "this")
        {
          outfile << "@" << "THIS" << endl;
        }
        else if (segment == "that")
        {
          outfile << "@" << "THAT" << endl;
        }

        outfile << "A=M" << endl;
        outfile << "D=A" << endl;
        outfile << "@" << index << endl;
        outfile << "D=D+A" << endl;

        // Spill *segment_base + index to R13
        outfile << "@" << "R13" << endl;
        outfile << "M=D" << endl;

        // update stack
        outfile << "@SP" << endl;
        outfile << "M=M-1" << endl;
        outfile << "A=M" << endl;
        outfile << "D=M" << endl;

        // Store in *segment_base + index
        outfile << "@" << "R13" << endl;
        outfile << "A=M" << endl;
        outfile << "M=D" << endl;
      }

      // See section 7.3.1 - Standard VM Mapping on the Hack Platform, Part 1
      else if (segment == "temp")
      {
        assert(index <= 8);

        outfile << "@SP" << endl;
        outfile << "M=M-1" << endl;
        outfile << "A=M" << endl;
        outfile << "D=M" << endl;
        outfile << "@" << "R" << 5 + index << endl;
        outfile << "M=D" << endl;
      }
      else if (segment == "static")
      {
        int staticAddress = STATIC_SEGMENT_BASE + index;

        outfile << "@SP" << endl;
        outfile << "M=M-1" << endl;
        outfile << "A=M" << endl;
        outfile << "D=M" << endl;
        outfile << "@" << staticAddress << endl;
        outfile << "M=D" << endl;
      }
      else if (segment == "pointer")
      {
        assert(index <= 2);

        string selectedRegister = (index == 0) ? "@THIS" : "@THAT";

        outfile << "@SP" << endl;
        outfile << "M=M-1" << endl;
        outfile << "A=M" << endl;
        outfile << "D=M" << endl;
        outfile << selectedRegister << endl;
        outfile << "M=D" << endl;
      }
      else
      {
        cerr << "pop " << segment << index;
        assert(0);
      }
    }
  }

  // pop top-most element from stack
  // if != zero, goto label
  // otherwise continue with next command
  void writeIfGoto(int lineNumber, Command_t command,
      string label)
  {
    if (command == C_IF_GOTO)
    {
      outfile << "// " << lineNumber << ": if-goto " << " " << label << endl;

      outfile << "@SP" << endl;
      outfile << "M=M-1" << endl;
      outfile << "A=M" << endl;
      outfile << "D=M" << endl;
      outfile << "@" << createBranchTag(label) << endl;
      outfile << "D;JNE" << endl;
    }
    else
    {
      assert(0);
    }
  }

  void writeLabel(int lineNumber, Command_t command,
      string label)
  {
    if (command == C_LABEL)
    {
      outfile << "// " << lineNumber << ": label " << " " << label << endl;

      outfile << "@" << createBranchTag(label) << endl;
    }
    else
    {
      assert(0);
    }
  }

  void writeGoto(int lineNumber, Command_t command,
      string label)
  {
    if (command == C_GOTO)
    {
      outfile << "// " << lineNumber << ": goto " << " " << label << endl;

      outfile << "@" << createBranchTag(label) << endl;
      outfile << "0;JMP" << endl;
    }
    else
    {
      assert(0);
    }
  }


};

class VMTranslator
{
  list<string> fileNameStemList;  // list .vm files with ".vm" dropped

  public:

  // VMTranslator - Populates `fileNameStemList` with one or more files
  // to be parsed and converted.
  VMTranslator(string argv)
  {
    struct stat argStat;
    bool isFile = false;

    // Determine if specified argument is a directory of filename

    int rvalue = stat(argv.c_str(), &argStat);

    if (rvalue == 0)
    {
      if (argStat.st_mode & S_IFDIR)
      {
        // isFile = false;
      }
      else if (argStat.st_mode & S_IFREG)
      {
        isFile = true;
      }
      else
      {
        cerr << "Not a file or directory." << endl;
        exit(-1);
      }
    }
    else
    {
      cerr << "File or directory not found." << endl;
      exit(-1);
    }

    if (isFile)
    {
      string filenameStem = argv.substr(0, argv.length() - 3);

      if ((argv.length() <= 3) || (argv.substr(argv.length() - 3) != ".vm"))
      {
        cerr << "Input filename required to end with .vm extension." << endl;
        exit(-1);
      }

      fileNameStemList.push_back(filenameStem);
    }
    else
    {
#ifdef CPP17_LATER
      namespace fs = std::filesystem;

      std::string path = "path_to_directory";
      for (auto & p : fs::directory_iterator(path))
        std::cout << p << std::endl;
#endif
      DIR* dir = opendir(argv.c_str());

      if (dir == NULL)
      {
        cerr << "Unable to read files in directory." << endl;
        exit(-1);
      }

      while(true)
      {
        struct dirent* dirEntry;
       
        dirEntry = readdir(dir);

        if (dirEntry == NULL)
        {
          if (errno != 0)
          {
            cerr << "Unhandled error while reading directory." << endl;
            exit(-1);
          }

          break;
        }

        if (strlen(dirEntry->d_name) <= 3)
          continue;

        string fileEntry(dirEntry->d_name);

        if (fileEntry.substr(fileEntry.length() - 3) != ".vm")
          continue;

        string filePath = argv + "/" + dirEntry->d_name;

        rvalue = stat(filePath.c_str(), &argStat);
        bool isFile = false;

        if (rvalue == 0)
        {
          if (argStat.st_mode & S_IFDIR)
          {
            cerr << "Directory contains an unsupported directory name." << endl;
            exit(-1);
          }
          else if (argStat.st_mode & S_IFREG)
          {
            isFile = true;
          }
          else
          {
            cerr << "Not a file or directory." << endl;
            exit(-1);
          }
        }
        else
        {
          cerr << "Directory contains an unsupported file." << endl;
          cerr << strerror(errno) << endl;
          exit(-1);
        }

        if (!isFile)
        {
          cerr << "Directory contains an unsupported filetype." << endl;
          exit(-1);
        }

        // They entry dirEntry->d_name is now considered okay

        string filenameStem = fileEntry.substr(0, fileEntry.length() - 3);
        fileNameStemList.push_back(argv + "/" + filenameStem);
      }
    }
  }

  void process()
  {
    for (auto filenameStem : fileNameStemList)
    {
      Parser parser(filenameStem);
      CodeWriter writer(filenameStem);

      while (parser.hasMoreCommands())
      {
        parser.advance();
        auto cmdType = parser.commandType();

        if (cmdType == C_ARITHMETIC)
        {
          writer.writeArithmetic(parser.lineNumber(), parser.arg1());
        }
        else if ((cmdType == C_PUSH) || (cmdType == C_POP))
        {
          writer.writePushPop(parser.lineNumber(), parser.commandType(),
              parser.arg1(), parser.args());
        }
        else if ((cmdType == C_IF_GOTO))
        {
          writer.writeIfGoto(parser.lineNumber(), parser.commandType(),
              parser.arg1()));
        }
        else if ((cmdType == C_LABEL))
        {
          writer.writeLabel(parser.lineNumber(), parser.commandType(),
              parser.arg1()));
        }
        else if ((cmdType == C_GOTO))
        {
          writer.writeGoto(parser.lineNumber(), parser.commandType(),
              parser.arg1()));
        }
      }
    }
  }
};

int main(int argc, char** argv)
{
  if (argc != 2)
  {
    cout << "USAGE: vmt [-h] FILENAME.vm | DIRECTORY" << endl;
    return 1;
  }

  if (strcmp(argv[1], "-h") == 0)
  {
    cout << "USAGE:\n\n"
              << "    vmt FILENAME.vm\n\n"
              << "    vmt DIRECTORY\n\n"
              << "DESCRIPTION\n\n"
              << "    Parses the VM commands found in FILENAME.vm into the corresponding Hack\n"
              << "    assembly code file, FILENAME.hack.  When provided the argument DIRECTORY,\n"
              << "    all .vm files will be processed as if called individually.\n" << endl;
    return 0;
  }

  VMTranslator vmTranslator(argv[1]);

  vmTranslator.process();

  return 0;
}
