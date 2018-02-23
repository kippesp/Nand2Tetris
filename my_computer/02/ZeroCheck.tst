// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/01/Mux16.tst

load ZeroCheck.hdl,
output-file ZeroCheck.out,
compare-to ZeroCheck.cmp,
output-list in%B1.16.1 out%B3.1.3;

set in 0,
eval,
output;

set in 1,
eval,
output;

set in %B0000000000000000,
eval,
output;

set in %B0000000000000001,
eval,
output;

set in %B0000000000000010,
eval,
output;

set in %B1000000000000000,
eval,
output;

set in %B0100000000000000,
eval,
output;

set in %B0010000000000000,
eval,
output;

set in %B0001000000000000,
eval,
output;

set in %B0000100000000000,
eval,
output;

set in %B0000010000000000,
eval,
output;

set in %B0000001000000000,
eval,
output;

set in %B0000000100000000,
eval,
output;

set in %B0000000010000000,
eval,
output;

set in %B0000000001000000,
eval,
output;

set in %B0000000000100000,
eval,
output;

set in %B0000000000010000,
eval,
output;

set in %B0000000000001000,
eval,
output;

set in %B0000000000000100,
eval,
output;

set in %B0000000000000010,
eval,
output;

set in %B0000000000000001,
eval,
output;

set in %B1001100001110110,
eval,
output;

set in %B1111111111111111,
eval,
output;
