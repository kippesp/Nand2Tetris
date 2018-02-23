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
// START:
//      Is keyboard pressed?
//          yes:
//              R0 <-- all black
//              goto FILL_SCREEN
//          no:
//              R0 <-- all white
// FILL_SCREEN:
//      R1 <-- base_screen_address
// DO_FILL:
//      MEM[R1] <-- R0
//      Is R1 > last_screen_word (16384 + 256*512/16 - 1)
//          no:
//              R1 <-- R1 + 1
//              goto DO_FILL
//          yes:
//              goto START

(START)
    @KBD
    D=M
    @SET_WHITE
    D;JEQ   // jmp if no key is pressed
    D=-1    // black
    @R0
    M=D
    @FILL_SCREEN
    0;JMP   // start the fill
(SET_WHITE)
    @R0
    M=0
(FILL_SCREEN)
    @SCREEN
    D=A
    @R1
    M=D     // base address of screen for first write
(DO_FILL)
    @R0
    D=M     // D==color
    @R1
    A=M     // load screen address from R1 into A
    M=D     // update 16 pixels in screen

    @R1
    D=M     // update screen address
    D=D+1   // to next address
    @R1
    M=D

    @24575  // Last screen address
    D=A
    @R1
    D=D-M
    @DO_FILL
    D;JGE   // continue if not done

    @START
    0;JMP   // restart program
