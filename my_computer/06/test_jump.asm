// Test for "D-1;JGE" instruction
(INIT)
    @10
    D=A
    @COUNTER
    M=D         // COUNTER := 10
    @RES1
    M=0         // RES := 0
(DEC)
    @COUNTER
    D=M
    @RES1
    M=D+M       // RES := RES + COUNTER
    @COUNTER
    MD=M-1      // COUNTER := COUNTER - 1
    @DEC
    D-1;JGE     // JMP IF COUNTER > 0
(STORE)
    @RES1
    @HALT
(HALT)
    0;JMP
