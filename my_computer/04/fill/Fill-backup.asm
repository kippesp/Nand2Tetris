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

// screen size is 256x512 pixels
// screen address is SCREEN (one pixel per word)
// 16384 bytes pre screen image

// Was going to rewrite to this....
//
// INIT:
//      R0 <-- 0        // 8 white pixels
// FILL_SCREEN:
//      R1 <-- 8192-1  // 16K screen
// LOOP:
//      SCREEN[R1] <-- R0
//      R1 <-- R1 - 1
//      Is R1 >= 0?
//          yes:
//              jmp LOOP
// CHECK_KEYBOARD:
//      Is keyboard pressed?
//          yes:
//              R0 <-- 16384    // 16 black pixels
//          no:
//              R0 <-- 0        // 16 white pixels
//      jmp FILL_SCREEN

(LOOP)
    @KBD    // read keyboard
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
    @SCREEN
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
