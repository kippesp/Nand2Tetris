// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Fill.asm

// Runs an infinite loop that listens to the keyboard input.
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel;
// the screen should remain fully black as long as the key is pressed. 
// When no key is pressed, the program clears the screen, i.e. writes
// "white" in every pixel;
// the screen should remain fully clear as long as no key is pressed.

// Put your code here.


    @24576  // read keyboard
    D=M
    @9
    D;JEQ   // no keypress? set to white color
    D=-1
    @0
    M=D
    @13
    0;JMP
    @0
    D=A
    @0
    M=D
    @16384
    D=A
    @1
    M=D     // set screen base address
    @0     // load color
    D=M
    @1
    A=M     // load screen address
    M=D     // update screen pixels
    @1
    D=M
    D=D+1   // increment screen address
    @1
    M=D
    @24575  // last screen location
    D=A
    @1
    D=D-M
    @17
    D;JGE   // continue if not done
    @0
    0;JMP   // restart program
