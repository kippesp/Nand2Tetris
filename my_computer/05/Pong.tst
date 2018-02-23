// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/05/ComputerMax-external.tst

load Computer.hdl,
output-file Pong.out,
//compare-to ComputerMax-external.cmp,
output-list time%S1.4.1;

// Load a program written in the Hack machine language.
// The program computes the maximum of RAM[0] and RAM[1] 
// and writes the result in RAM[2].
ROM32K load Pong.hack,

// first run: compute max(3,5)
//set RAM16K[0] 3,
//set RAM16K[1] 5,
//output;

repeat 1000000 {
    tick, tock, output;
}
