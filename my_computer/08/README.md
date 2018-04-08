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

## Stage 2

See: http://www.nand2tetris.org/08.php

### Parser Module

Handles parsing of .vm file.

#### Added support

Parse LABEL, GOTO, IF, FUNCTION, RETURN, and CALL command types.

### CodeWriter Module

Translates VM commands into Hack assembly.

#### Added methods

- void writeInit()
- void writeLabel(string)
- void writeGogo(string)
- void writeIf(string)
- void writeCall(string, int)
- void writeReturn()
- void writeFunction(string, int)

### Test Programs

- ProgramFlow/BasicLoop
- ProgramFlow/FibonacciSeries
- FunctionCalls/SimpleFunction
- FunctionCalls/NestedCall
- FunctionCalls/FibonacciElement
- FunctionCalls/StaticsTest
