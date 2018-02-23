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


(LOOP)
    @24576  // read keyboard
    D=M
    @WHITE
    D;JEQ   // no keypress? set to white color
    D=-1
    @R0
    M=D
    @DOFILL
    0;JMP
(WHITE)
    @0
    D=A
    @R0
    M=D
(DOFILL)
    @16384
    D=A
    @R1
    M=D     // set screen base address
(FILOOP)
    @R0     // load color
    D=M
    @R1
    A=M     // load screen address
    M=D     // update screen pixels

    @R1
    D=M
    D=D+1   // increment screen address
    @R1
    M=D
    @24575  // last screen location
    D=A
    @R1
    D=D-M
    @FILOOP
    D;JGE   // continue if not done

    @LOOP
    0;JMP   // restart program
