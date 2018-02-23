#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <list>

// push constant N
// pop
// add
// sub
// neg
// eq
// gt
// lt
// and
// or
// not

using namespace std;

typedef enum {
  C_NONE,
  C_ARITHMETIC,
  C_PUSH,
  C_POP,
  C_LABEL,
  C_GOTO,
  C_IF,
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
  Parser(string filename) : commandLineNumber(0)
  {
    ifstream infile;

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

public:

  CodeWriter(string outfilename) 
  {
    outfile.open(outfilename, ofstream::out);

    if (!outfile.is_open())
    {
      cerr << "Failed to open output file, " << outfilename << endl;
      exit(-2);
    }
  }

  void setFileName(string filename) {}

  // See section 7.2.3 - Memory Access Commands
  void writeArithmetic(int lineNumber, string command)
  {
    // add - binary
    if (command == "add")
    {
      outfile << "// " << lineNumber << ": add" << endl;
      outfile << "@SP" << endl;
      outfile << "M=M-1" << endl;
      outfile << "D=M" << endl;
      outfile << "M=M-1" << endl;
      outfile << "D=D+M" << endl;
      outfile << "M=D" << endl;
      outfile << "M=M+1" << endl;
    }

    // add - binary
    if (command == "sub")
    {
      outfile << "// " << lineNumber << ": sub" << endl;
      outfile << "@SP" << endl;
      outfile << "M=M-1" << endl;
      outfile << "D=M" << endl;
      outfile << "M=M-1" << endl;
      outfile << "D=D-M" << endl;
      outfile << "M=D" << endl;
      outfile << "M=M+1" << endl;
    }
  }

  // See section 7.2.3 - Memory Access Commands
  void writePushPop(int lineNumber, Command_t command, string segment,
      int index)
  {
    if (command == C_PUSH)
    {
      outfile << "// " << lineNumber << ": push " << segment << " " << index << endl;

      // TODO: Clean up repetition

      if (segment == "constant")
      {
        outfile << "@" << index << endl;
        outfile << "D=A" << endl;
        outfile << "@SP" << endl;
        outfile << "M=D" << endl;
        outfile << "M=M+1" << endl;
      }
      else if (segment == "local")
      {
        // D <-- [Local] + index
        outfile << "@" << "LCL" << endl;
        outfile << "D=M" << endl;
        outfile << "@" << index << endl;
        outfile << "D=D+A" << endl;
        outfile << "A=D" << endl;

        // push D onto stack
        outfile << "@" << "SP" << endl;
        outfile << "M=D" << endl;
        outfile << "A=M+1" << endl;
        outfile << "M=A" << endl;
      }
      else if (segment == "that")
      {
        // D <-- [that] + index
        outfile << "@" << "THAT" << endl;
        outfile << "D=M" << endl;
        outfile << "@" << index << endl;
        outfile << "D=D+A" << endl;
        outfile << "A=D" << endl;

        // push D onto stack
        outfile << "@" << "SP" << endl;
        outfile << "M=D" << endl;
        outfile << "A=M+1" << endl;
        outfile << "M=A" << endl;
      }
      else if (segment == "this")
      {
        // D <-- [this] + index
        outfile << "@" << "THIS" << endl;
        outfile << "D=M" << endl;
        outfile << "@" << index << endl;
        outfile << "D=D+A" << endl;
        outfile << "A=D" << endl;

        // push D onto stack
        outfile << "@" << "SP" << endl;
        outfile << "M=D" << endl;
        outfile << "A=M+1" << endl;
        outfile << "M=A" << endl;
      }
      else if (segment == "argument")
      {
        // D <-- [argument] + index
        outfile << "@" << "ARG" << endl;
        outfile << "D=M" << endl;
        outfile << "@" << index << endl;
        outfile << "D=D+A" << endl;
        outfile << "A=D" << endl;

        // push D onto stack
        outfile << "@" << "SP" << endl;
        outfile << "M=D" << endl;
        outfile << "A=M+1" << endl;
        outfile << "M=A" << endl;
      }
      else if (segment == "temp")
      {
        assert(index <= 8);

        outfile << "@" << "R" << 5 + index << endl;
        outfile << "D=M" << endl;

        // push D onto stack
        outfile << "@" << "SP" << endl;
        outfile << "M=D" << endl;
        outfile << "A=M+1" << endl;
        outfile << "M=A" << endl;
      }
      else
      {
        assert(0);
      }
    }
    else if (command == C_POP)
    {
      outfile << "// " << lineNumber << ": pop " << segment << " " << index << endl;

      // Compute destination address, [SEGMENT + index]
      if (segment == "local")
      {
        outfile << "@" << "LCL" << endl;
      }
      else if (segment == "argument")
      {
        outfile << "@" << "ARG" << endl;
      }
      else if (segment == "that")
      {
        outfile << "@" << "THAT" << endl;
      }
      else if (segment == "this")
      {
        outfile << "@" << "THIS" << endl;
      }
      // See section 7.3.1 - Standard VM Mapping on the Hack Platform, Part 1
      else if (segment == "temp")
      {
        assert(index <= 8);

        outfile << "@" << "R" << 5 + index << endl;
      }
      else
      {
        assert(0);
      }

      outfile << "D=M" << endl;

      // The TEMP segment is defined as R5 - R12
      if (segment != "temp")
      {
        outfile << "@" << index << endl;
        outfile << "D=D+A" << endl;
      }

      // Temporarily put destination address at SP
      outfile << "@" << "SP" << endl;
      outfile << "M=D" << endl;

      // Decrement SP and load value to store from SP+1
      outfile << "@SP" << endl;
      outfile << "M=M-1" << endl;
      outfile << "D=M" << endl;

      // Store value to destination address
      outfile << "A=M+1" << endl;
      outfile << "M=D" << endl;
    }
  }

/*
  void Close() {}
*/
};

int main(int argc, char** argv)
{
  if (argc != 2)
  {
    cout << "USAGE: vmt FILENAME.vm" << endl;
    return 1;
  }

  if (strcmp(argv[1], "-h") == 0)
  {
    cout << "USAGE:\n\n"
              << "    vmt FILENAME.vm\n\n"
              << "DESCRIPTION\n\n"
              << "    Parses the VM commands found in FILENAME.vm into the corresponding Hack\n"
                 "    assembly code file, FILENAME.hack" << endl;
    return 0;
  }

  string infilename = argv[1];
  string filenameStem = infilename.substr(0, infilename.length() - 3);

  if (infilename.substr(infilename.length() - 3) != ".vm")
  {
    cerr << "Input filename required to end with .vm extension." << endl;
    return -1;
  }

  Parser parser(argv[1]);
  CodeWriter writer(filenameStem + ".asm");

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

    /*
    cout << parser.lineNumber() << " :";
    cout << cmdType << " ";
    cout << parser.arg1() << " ";

    if ((cmdType == C_PUSH) || (cmdType == C_POP) ||
        (cmdType == C_FUNCTION) || (cmdType == C_CALL))
    {
      cout << parser.args();
    }

    cout << endl;
    */

  }

  return 0;
}
