#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <list>
#include <libgen.h>
#include <set>
#include <sys/stat.h>
#include <stdlib.h>

#ifdef CPP17_LATER
# include <filesystem>
#else
# include <dirent.h>
#endif

#ifndef NDEBUG
# define ASSERT(condition, message) \
    do { \
        if (! (condition)) { \
            std::cerr << "Assertion `" #condition "` failed in " << __FILE__ \
                      << " line " << __LINE__ << ": " << message << std::endl; \
            std::terminate(); \
        } \
    } while (false)
#else
# define ASSERT(condition, message) do { } while (false)
#endif


// TODO: Add metrics.

// R13-R15 GP registers

using namespace std;

typedef enum {
  C_NONE,
  C_ARITHMETIC,
  C_PUSH,
  C_POP,
  C_LABEL,
  C_GOTO,
  C_IF_GOTO,
  C_FUNCTION,
  C_CALL,
  C_RETURN,
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
  Parser(string pathname) : commandLineNumber(0)
  {
    ifstream infile;

    infile.open(pathname, ios::in);

    if (!infile.is_open())
    {
      cerr << "Failed to open input file, " << pathname << endl;
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

      // ignore line if first two characters are '//'
      if (line.substr(0, 2) == "//")
        continue;

      // drop ending CRLF character
      if (line.substr(line.length() - 1, 1) == "\r")
        line = line.substr(start, line.length() - 1);

      // ignore empty lines
      if (line.substr(0, 1) == "")
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

#ifdef NONO
    // TODO: make smarter - don't transform labels of function names
    transform(currentLine.begin(), currentLine.end(), currentLine.begin(),
        ::tolower);
#endif

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
  
    if (command == "push")     return C_PUSH;
    if (command == "pop")      return C_POP;
    if (command == "eq")       return C_ARITHMETIC;
    if (command == "lt")       return C_ARITHMETIC;
    if (command == "gt")       return C_ARITHMETIC;
    if (command == "add")      return C_ARITHMETIC;
    if (command == "sub")      return C_ARITHMETIC;
    if (command == "neg")      return C_ARITHMETIC;
    if (command == "and")      return C_ARITHMETIC;
    if (command == "or")       return C_ARITHMETIC;
    if (command == "not")      return C_ARITHMETIC;
    if (command == "label")    return C_LABEL;
    if (command == "if-goto")  return C_IF_GOTO;
    if (command == "goto")     return C_GOTO;
    if (command == "function") return C_FUNCTION;
    if (command == "return")   return C_RETURN;
    if (command == "call")     return C_CALL;

    ASSERT(0, string("Unsupported command: ") + command);

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

    if ((cmdType == C_LABEL) || (cmdType == C_IF_GOTO) || (cmdType == C_GOTO) ||
        (cmdType == C_FUNCTION) || (cmdType == C_CALL))
    {
      string label = *itr;

#ifdef NONO
      // Make all labels upper case
      transform(label.begin(), label.end(), label.begin(), ::toupper);
#endif

      return label;
    }

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
  const string outputFilenameStem = "unknown";
  const string outputFilename = "unknown";
  string currentInputFilenameStem = "unset";
  string currentFunction = "anonymous";
  set<string> fileLabels;
  int anonymousLabelCounter = 0;
  int returnLabelCounter = 0;

  string getLabel(string label)
  {
      string newLabel;

      // Use for call return labels
      if (label == "")
      {
          anonymousLabelCounter++;
          assert(0);
          newLabel = string(currentFunction) + "$ret." + to_string(anonymousLabelCounter);
      }
      // Use for branching labels
      else
      {
          newLabel = label;
      }

      return newLabel;
  }

  string newLabel(string label)
  {
      string newLabel = getLabel(label);

      if (fileLabels.find(newLabel) != fileLabels.end())
      {
          cerr << "Duplicate label: " << newLabel << endl;
          exit(-1);
      }

      fileLabels.insert(newLabel);

      return newLabel;
  }

  string createReturnLabel()
  {
    string newLabel = string(currentFunction) + "$ret." + to_string(returnLabelCounter);
    returnLabelCounter++;

    return newLabel;
  }

public:

  CodeWriter(string filenameStem) : outputFilenameStem(filenameStem),
                                    outputFilename(filenameStem + ".asm")
  {
    outfile.open(outputFilename, ofstream::out);

    if (!outfile.is_open())
    {
      cerr << "Failed to open output file, " << outputFilename << endl;
      exit(-2);
    }
  }

  ~CodeWriter()
  {
    outfile.close();
  }

  void setInputFilenameStem(string inputFilenameStem)
  {
    currentInputFilenameStem = inputFilenameStem;
    outfile << "// File: " << currentInputFilenameStem + ".vm" << endl;
  }

  void writeInit()
  {
    outfile << "// Bootstrap code to Sys.init function" << endl << endl;

    outfile << "@" << 256 << endl;
    outfile << "D=A" << endl;
    outfile << "@SP" << endl;
    outfile << "M=D" << endl;

    outfile << "// For debug, set LCL=-1, ARG=-2, THIS=-3, THAT=-4" << endl;
    outfile << "D=-1" << endl;
    outfile << "@LCL" << endl;
    outfile << "M=D" << endl;

    outfile << "D=D-1" << endl;
    outfile << "@ARG" << endl;
    outfile << "M=D" << endl;

    outfile << "D=D-1" << endl;
    outfile << "@THIS" << endl;
    outfile << "M=D" << endl;

    outfile << "D=D-1" << endl;
    outfile << "@THAT" << endl;
    outfile << "M=D" << endl;

    writeCall(0, C_CALL, "Sys.init", 0);
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
      outfile << "AM=M-1" << endl;
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
      outfile << "AM=M-1" << endl;// SP  = SP'-1
      outfile << "D=M" << endl;   // D   = *[SP'-1] = *[SP'-2]
      outfile << "A=A-1" << endl; // A   = SP'-2
      outfile << "D=M-D" << endl; // D   = *[SP'-2] - *[SP'-1]

      outfile << "@CMP_" << branchNumber << endl;

      if (command == "eq")
        outfile << "D;JEQ" << endl;
      else if (command == "lt")
        outfile << "D;JLT" << endl;
      else
        outfile << "D;JGT" << endl;

      outfile << "D=0" << endl;
      outfile << "@JOIN_CMP_" << branchNumber << endl;
      outfile << "0;JMP" << endl;
      outfile << "(CMP_" << branchNumber << ")" << endl;
      outfile << "D=-1" << endl;
      outfile << "(JOIN_CMP_" << branchNumber << ")" << endl;

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
        outfile << "AD=D+A" << endl;
        outfile << "D=M" << endl;

        // push D onto stack
        outfile << "@SP" << endl;
        outfile << "AM=M+1" << endl;
        outfile << "A=A-1" << endl;
        outfile << "M=D" << endl;
      }
      else if (segment == "constant")
      {
        outfile << "@" << index << endl;
        outfile << "D=A" << endl;

        // push D onto stack
        outfile << "@SP" << endl;
        outfile << "AM=M+1" << endl;
        outfile << "A=A-1" << endl;
        outfile << "M=D" << endl;
      }
      else if (segment == "temp")
      {
        assert(index <= 8);

        outfile << "@" << "R" << 5 + index << endl;
        outfile << "D=M" << endl;

        // push D onto stack
        outfile << "@SP" << endl;
        outfile << "AM=M+1" << endl;
        outfile << "A=A-1" << endl;
        outfile << "M=D" << endl;
      }
      else if (segment == "static")
      {
        outfile << "@" << currentInputFilenameStem << "." << index << endl;
        outfile << "D=M" << endl;

        // push D onto stack
        outfile << "@SP" << endl;
        outfile << "AM=M+1" << endl;
        outfile << "A=A-1" << endl;
        outfile << "M=D" << endl;
      }
      else if (segment == "pointer")
      {
        assert(index <= 2);

        string selectedRegister = (index == 0) ? "@THIS" : "@THAT";

        outfile << selectedRegister << endl;

        outfile << "D=M" << endl;

        // push D onto stack
        outfile << "@SP" << endl;
        outfile << "AM=M+1" << endl;
        outfile << "A=A-1" << endl;
        outfile << "M=D" << endl;
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

        outfile << "AD=M" << endl;
        outfile << "@" << index << endl;
        outfile << "D=D+A" << endl;

        // Spill *segment_base + index to R15
        outfile << "@" << "R15" << endl;
        outfile << "M=D" << endl;

        // update stack
        outfile << "@SP" << endl;
        outfile << "AM=M-1" << endl;
        outfile << "D=M" << endl;

        // Store in *segment_base + index
        outfile << "@" << "R15" << endl;
        outfile << "A=M" << endl;
        outfile << "M=D" << endl;
      }

      // See section 7.3.1 - Standard VM Mapping on the Hack Platform, Part 1
      else if (segment == "temp")
      {
        assert(index <= 8);

        outfile << "@SP" << endl;
        outfile << "AM=M-1" << endl;
        outfile << "D=M" << endl;
        outfile << "@" << "R" << 5 + index << endl;
        outfile << "M=D" << endl;
      }
      else if (segment == "static")
      {
        outfile << "@SP" << endl;
        outfile << "AM=M-1" << endl;
        outfile << "D=M" << endl;
        outfile << "@" << currentInputFilenameStem << "." << index << endl;
        outfile << "M=D" << endl;
      }
      else if (segment == "pointer")
      {
        assert(index <= 2);

        string selectedRegister = (index == 0) ? "@THIS" : "@THAT";

        outfile << "@SP" << endl;
        outfile << "AM=M-1" << endl;
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
      outfile << "@" << getLabel(label) << endl;
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
      string newLabel = getLabel(label);

      outfile << "// " << lineNumber << ": label " << newLabel << endl;

      outfile << "(" << newLabel << ")" << endl;
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
      string gotoLabel = label;

      outfile << "// " << lineNumber << ": goto " << gotoLabel << endl;

      outfile << "@" << gotoLabel << endl;
      outfile << "0;JMP" << endl;
    }
    else
    {
      assert(0);
    }
  }

  // On entry:
  //   Segment local has been initialized to zeros for each local variable
  //   Segment static has been set
  //   Segments this, that, pointer, and temp are undefined
  // On exit:
  //   Return value is pushed onto stack.
  void writeFunction(int lineNumber, Command_t command,
      string label, int nargs)
  {
    if (command == C_FUNCTION)
    {
      anonymousLabelCounter = 0;
      returnLabelCounter = 0;
      currentFunction = label;

      // TODO: Validate function name (pg. 160)
      // dot, letters, digits, underscores, color; can not start with digit
      outfile << "// " << lineNumber << ": function" << " " << label;
      outfile << " (" << nargs << " nargs)" << endl;

      outfile << "(" << label << ")" << endl;

      // Build local variables
      for (int i = 0; i < nargs; i++)
      {
        writePushPop(lineNumber, C_PUSH, "constant", 0);
      }
    }
    else
    {
      assert(0);
    }
  }

  void writeCall(int lineNumber, Command_t command,
      string label, int nargs)
  {
    if (command == C_CALL)
    {
      // TODO: Validate function name (pg. 160)
      // dot, letters, digits, underscores, color; can not start with digit
      outfile << "// " << lineNumber << ": call" << " " << label;
      outfile << " (" << nargs << " nargs)" << endl;

      // create label for the return goto
      string returnAddressLabel = createReturnLabel();

      // load address of returnAddressLabel
      outfile << "@" << returnAddressLabel << endl;
      outfile << "D=A" << endl;

      // push address of returnAddressLabel onto stack
      outfile << "@SP" << endl;
      outfile << "AM=M+1" << endl;
      outfile << "A=A-1" << endl;
      outfile << "M=D" << endl;

      // push LCL
      outfile << "@LCL" << endl;
      outfile << "D=M" << endl;
      outfile << "@SP" << endl;
      outfile << "AM=M+1" << endl;
      outfile << "A=A-1" << endl;
      outfile << "M=D" << endl;

      // push ARG
      outfile << "@ARG" << endl;
      outfile << "D=M" << endl;
      outfile << "@SP" << endl;
      outfile << "AM=M+1" << endl;
      outfile << "A=A-1" << endl;
      outfile << "M=D" << endl;

      // push THIS
      outfile << "@THIS" << endl;
      outfile << "D=M" << endl;
      outfile << "@SP" << endl;
      outfile << "AM=M+1" << endl;
      outfile << "A=A-1" << endl;
      outfile << "M=D" << endl;

      // push THAT
      outfile << "@THAT" << endl;
      outfile << "D=M" << endl;
      outfile << "@SP" << endl;
      outfile << "AM=M+1" << endl;
      outfile << "A=A-1" << endl;
      outfile << "M=D" << endl;

      // point ARG to arg0
      // ARG = SP - 5 - nargs  // point to arg0
      outfile << "@SP" << endl;
      outfile << "D=M" << endl;
      outfile << "@5" << endl;
      outfile << "D=D-A" << endl;
      outfile << "@" << nargs << endl;
      outfile << "D=D-A" << endl;
      outfile << "@ARG" << endl;
      outfile << "M=D" << endl;

      // reposition LCL
      // LCL = SP
      outfile << "@SP" << endl;
      outfile << "D=M" << endl;
      outfile << "@LCL" << endl;
      outfile << "M=D" << endl;

      writeGoto(lineNumber, C_GOTO, label);

      outfile << "(" << returnAddressLabel << ")" << endl;
    }
    else
    {
      assert(0);
    }
  }

  void writeReturn(int lineNumber, Command_t command)
  {
    if (command == C_RETURN)
    {
      outfile << "// " << lineNumber << ": return" << endl;

      // R13 - LCL
      // R14 - Return Address

      // R13 = LCL - Save LCL (endFrame) to temporary register
      outfile << "// " << lineNumber << ": R13 = FRAME = LCL" << endl;
      outfile << "@LCL" << endl;
      outfile << "D=M" << endl;
      outfile << "@R13" << endl;
      outfile << "M=D" << endl;
      // R14 = *(endFrame - 5) = *(R13 - 5) - Save return address in R14
      outfile << "// " << lineNumber << ": R14 = RET = *(FRAME-5)" << endl;
      outfile << "@R13" << endl;
      outfile << "D=M" << endl;
      outfile << "@5" << endl;
      outfile << "A=D-A" << endl;
      outfile << "D=M" << endl;
      outfile << "@R14" << endl;
      outfile << "M=D" << endl;
      // *ARG = pop() - Reposition the return value for caller
      outfile << "// " << lineNumber << ": *ARG = pop()" << endl;
      writePushPop(lineNumber, C_POP, "argument", 0); // trashes R15
      // SP = ARG+1 - Restore SP of caller
      outfile << "// " << lineNumber << ": SP = ARG+1" << endl;
      outfile << "@ARG" << endl;
      outfile << "AD=M+1" << endl;
      outfile << "@SP" << endl;
      outfile << "M=D" << endl;
      // THAT = *(R13 - 1) - Restore THAT of caller
      outfile << "// " << lineNumber << ": THAT = *(FRAME-1)" << endl;
      outfile << "@R13" << endl;
      outfile << "A=M" << endl;
      outfile << "A=A-1" << endl;
      outfile << "D=M" << endl;
      outfile << "@THAT" << endl;
      outfile << "M=D" << endl;
      // THIS = *(R13 - 2) - Restore THIS of caller
      outfile << "// " << lineNumber << ": THIS = *(FRAME-2)" << endl;
      outfile << "@R13" << endl;
      outfile << "A=M" << endl;
      outfile << "A=A-1" << endl;
      outfile << "A=A-1" << endl;
      outfile << "D=M" << endl;
      outfile << "@THIS" << endl;
      outfile << "M=D" << endl;
      // ARG = *(R13 - 3) - Restore ARG of caller
      outfile << "// " << lineNumber << ": ARG = *(FRAME-3)" << endl;
      outfile << "@R13" << endl;
      outfile << "D=M" << endl;
      outfile << "@3" << endl;
      outfile << "A=D-A" << endl;
      outfile << "D=M" << endl;
      outfile << "@ARG" << endl;
      outfile << "M=D" << endl;
      // LCL = *(R13 - 4) - Restore LCL of caller
      outfile << "// " << lineNumber << ": LCL = *(FRAME-4)" << endl;
      outfile << "@R13" << endl;
      outfile << "D=M" << endl;
      outfile << "@4" << endl;
      outfile << "A=D-A" << endl;
      outfile << "D=M" << endl;
      outfile << "@LCL" << endl;
      outfile << "M=D" << endl;
      // goto RET (R14)
      outfile << "// " << lineNumber << ": goto RET/R14" << endl;
      outfile << "@R14" << endl;
      outfile << "A=M" << endl;
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
  string directoryName;
  string outputFilenameStem;
  bool bootstrapRequired = false;

  public:

  // VMTranslator - Populates `fileNameStemList` with one or more files
  // to be parsed and converted.
  VMTranslator(string argv)
  {
    struct stat argStat;
    bool isFile = false;
    char tmpPath[PATH_MAX + 1];

    // Determine if specified argument is a directory of filename

    int rvalue = stat(argv.c_str(), &argStat);

    if (rvalue == 0)
    {
      if ((argStat.st_mode & S_IFDIR) == 0)
      {
        FILE* infile = fopen(argv.c_str(), "r");

        if (infile != nullptr)
        {
          isFile = true;
          fclose(infile);
        }
        else
        {
          cerr << "Error opening file." << endl;
          exit(-1);
        }
      }
    }
    else
    {
      cerr << "File or directory not found." << endl;
      exit(-1);
    }

    char* rvalue_s;
   
    rvalue_s = dirname_r(argv.c_str(), tmpPath);

    if (rvalue_s == nullptr)
    {
      puts("Failed to get directory path");
      exit(1);
    }

    if (isFile)
    {
      directoryName = string(tmpPath);

      // Extract the output filename from provided path
      rvalue_s = basename_r(argv.c_str(), tmpPath);

      if (rvalue_s == nullptr)
      {
        puts("Failed to get final path component name");
        exit(1);
      }

      string tmpPath_s(tmpPath);

      if ((tmpPath_s.length() <= 3) ||
          (tmpPath_s.substr(tmpPath_s.length() - 3) != ".vm"))
      {
        cerr << "Input filename required to end with .vm extension." << endl;
        exit(-1);
      }

      outputFilenameStem = tmpPath_s.substr(0, tmpPath_s.length() - 3);
    }
    else
    {
      // Expand given directory into full path
      rvalue_s = realpath(argv.c_str(), tmpPath);

      if (rvalue_s == nullptr)
      {
        puts("Failed to get absolute directory path");
        exit(1);
      }

      directoryName = string(tmpPath);

      // Derive output filename from directory
      rvalue_s = basename_r(directoryName.c_str(), tmpPath);

      if (rvalue_s == nullptr)
      {
        puts("Failed to get final path component name");
        exit(1);
      }

      outputFilenameStem = string(tmpPath);
    }

    //
    // Add file(s) to be processed to fileNameStemList
    //

    if (isFile)
    {
      fileNameStemList.push_back(outputFilenameStem);
    }
    else
    {
      // Directories are assumed to require bootstrap code
      bootstrapRequired = true;

      DIR* dir = opendir(directoryName.c_str());

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

        string filePath = directoryName + "/" + dirEntry->d_name;

        rvalue = stat(filePath.c_str(), &argStat);

        if (rvalue == 0)
        {
          if (argStat.st_mode & S_IFDIR)
          {
            // ignore directories ending in .vm
          }
          else if (argStat.st_mode & S_IFREG)
          {
            string filenameStem = fileEntry.substr(0, fileEntry.length() - 3);
            fileNameStemList.push_back(filenameStem);
          }
          else
          {
            // ignore
          }
        }
        else
        {
          // ignore situation
        }
      }
    }

    if (fileNameStemList.size() == 0)
    {
      puts("No .vm files found.  Exiting normally.");
      exit(0);
    }
  }

  void process()
  {
    CodeWriter writer(directoryName + "/" + outputFilenameStem);

    if (bootstrapRequired)
    {
      writer.writeInit();
    }

    // TODO: If directory name set, open writer file with that name
    // TODO: Writer is only a single .asm file.  So either way,
    // can call writer just once.  Why the list of stems for writing?  Just needed for reading.

    for (auto filenameStem : fileNameStemList)
    {
      Parser parser(directoryName + "/" + filenameStem + ".vm");
      writer.setInputFilenameStem(filenameStem);

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
        else if (cmdType == C_IF_GOTO)
        {
          writer.writeIfGoto(parser.lineNumber(), parser.commandType(),
              parser.arg1());
        }
        else if (cmdType == C_LABEL)
        {
          writer.writeLabel(parser.lineNumber(), parser.commandType(),
              parser.arg1());
        }
        else if (cmdType == C_GOTO)
        {
          writer.writeGoto(parser.lineNumber(), parser.commandType(),
              parser.arg1());
        }
        else if (cmdType == C_FUNCTION)
        {
          writer.writeFunction(parser.lineNumber(), parser.commandType(),
              parser.arg1(), parser.args());
        }
        else if (cmdType == C_CALL)
        {
          writer.writeCall(parser.lineNumber(), parser.commandType(),
              parser.arg1(), parser.args());
        }
        else if (cmdType == C_RETURN)
        {
          writer.writeReturn(parser.lineNumber(), parser.commandType());
        }
        else
        {
          ASSERT(0, string("Unsupported cmdType."));
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
