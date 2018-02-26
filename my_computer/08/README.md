# Virtual Machine

## Usage

        vmt [-h] FILE.vm|DIRECTORY

    Parses the VM commands found in FILENAME.vm into the corresponding Hack
    assembly code file, FILENAME.hack.  When provided the argument DIRECTORY,
    all .vm files will be processed as if called individually.

## Pre-defined Registers

    - RAM[0] - SP   (stack pointer)
    - RAM[1] - LCL  (local segment)
    - RAM[2] - ARG  (argument segment)
    - RAM[3] - THIS (this segment)
    - RAM[4] - THAT (that segment)
    - RAM[5-12] -   (temp segment)
    - RAM[13-15] -  (general purpose registers)

## Stage 1

### Parser Module

Handles parsing of .vm file.

#### Recommended Methods

- void this(infile)
- bool hasMoreCommands()
- void advance()
- TYPE commandType()
- string arg1()
- int arg2()

### CodeWriter Module

Translates VM commands into Hack assembly.

#### Recommended Methods

- void this(outfile)
- void setFileName(string)
- void writeArithmetic(string)
- void WritePushPop(C_PUSH|C_POP, segment_string, index_int)
- void Close()

### Test Programs

- SimpleAdd
- StackTest
