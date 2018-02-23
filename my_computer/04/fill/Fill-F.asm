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


["    @24576  // read keyboard",                   "0110000000000000"],
["    D=M",                                        "1111110000010000"],
["    @9",                                         "0000000000001001"],
["    D;JEQ   // no keypress? set to white color", "1110001100000010"],
["    D=-1",                                       "1110111010010000"],
["    @0",                                         "0000000000000000"],
["    M=D",                                        "1110001100001000"],
["    @13",                                        "0000000000001101"],
["    0;JMP",                                      "1110101010000111"],
["    @0",                                         "0000000000000000"],
["    D=A",                                        "1110110000010000"],
["    @0",                                         "0000000000000000"],
["    M=D",                                        "1110001100001000"],
["    @16384",                                     "0100000000000000"],
["    D=A",                                        "1110110000010000"],
["    @1",                                         "0000000000000001"],
["    M=D     // set screen base address",         "1110001100001000"],
["    @0     // load color",                       "0000000000000000"],
["    D=M",                                        "1111110000010000"],
["    @1",                                         "0000000000000001"],
["    A=M     // load screen address",             "1111110000100000"],
["    M=D     // update screen pixels",            "1110001100001000"],
["    @1",                                         "0000000000000001"],
["    D=M",                                        "1111110000010000"],
["    D=D+1   // increment screen address",        "1110011111010000"],
["    @1",                                         "0000000000000001"],
["    M=D",                                        "1110001100001000"],
["    @24575  // last screen location",            "0101111111111111"],
["    D=A",                                        "1110110000010000"],
["    @1",                                         "0000000000000001"],
["    D=D-M",                                      "1111010011010000"],
["    @17",                                        "0000000000010001"],
["    D;JGE   // continue if not done",            "1110001100000011"],
["    @0",                                         "0000000000000000"],
["    0;JMP   // restart program",                 "1110101010000111"],
