// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Mult.asm

// Multiplies R0 and R1 and stores the result in R2.
// (R0, R1, R2 refer to RAM[0], RAM[1], and RAM[2], respectively.)

// Put your code here.



// Output register
    @R2
    M=0     // R2 <-- 0
// Check if already done
    @R1
    D=M     // D <-- multiplicand
    @END
    D;JEQ   // if (multiplicant == 0) then we're done
(LOOP)
    @R0
    D=M     // D <-- multiplier
    @END
    D;JEQ   // if (multiplier == 0) then we're done
    @R1
    D=M     // D <-- multiplicand
    @R2
    M=D+M   // add multiplicand to result
    @R0
    M=M-1   // decrement multiplier
    @LOOP
    0;JMP   // interate
(END)
    @END
    0;JMP   // HALT
