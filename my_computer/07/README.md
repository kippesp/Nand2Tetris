# Virtual Machine

## Usage

        VMtranslator [-h] FILE.vm|DIRECTORY

    When given a file, translate it into a single assembly language file,
    FILE.asm.  Or when given a directory, translate all the .vm files into .asm
    files.

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
