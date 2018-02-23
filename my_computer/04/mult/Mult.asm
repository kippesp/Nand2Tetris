// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Mult.asm

// Multiplies R0 and R1 and stores the result in R2.
// (R0, R1, R2 refer to RAM[0], RAM[1], and RAM[3], respectively.)

// Program: R2 <-- R0 * R1

// SETUP:
//   Is R0 > R1?
//      yes
//          R3 <-- R1   (smaller value)
//          R4 <-- R0
//      no
//          R3 <-- R0   (smaller value)
//          R4 <-- R1

// INIT:
//   R2 <-- 0
// CHECK_IF_DONE:
//   Is R3 == 0?
//      yes
//          finished, goto END
//      no
//          R2 <-- R2 + R4
//          R3 <-- R3 - 1
//          goto CHECK_IF_DONE
// END:
//   goto END

(SETUP)
//  Is R0 - R1 > 0?

//  A <-- R0 - R1
    @R0
    D=M
    @R1
    D=D-M
//  A > 0?
    @R0_IS_GREATER
    D;JGE
// R3 <-- R0 (smaller value)
// R4 <-- R1
(R1_IS_GREATER)
    @R0
    D=M
    @R3
    M=D

    @R1
    D=M
    @R4
    M=D
    @INIT
    0;JMP
// R3 <-- R1 (smaller value)
// R4 <-- R0
(R0_IS_GREATER)
    @R1
    D=M
    @R3
    M=D

    @R0
    D=M
    @R4
    M=D

(INIT)
// R2 <-- 0
    @R2
    M=0
// Is R3 == 0?
    @R3
    D=M
(CHECK_R3_IF_DONE)
    @END
    D;JEQ

// R2 <-- R2 + R4
    @R4
    D=M
    @R2
    M=D+M

// R3 <-- R3 - 1
    @R3
    M=M-1
    D=M
    @CHECK_R3_IF_DONE
    0;JMP

(END)
    @END
    0;JMP
