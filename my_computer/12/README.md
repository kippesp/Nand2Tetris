# Jack Standard Library

A functional implementation of the Jack OS.

Project 12 is on the whole uninteresting and I hadn't intended to implement as
much as I did.  But what I found from other implementations was unimpressive.

## Math.jack

An attempt was made to achieve a similar, perceptible speed to the reference
JackOS implementation's .vm files.  The book's recursive divide algorithm was
observed to be slow and redone using a faster O(1) implementation taken from an
old 6502 assembly language book (and aluded to in the Schocken book).
Additionally, Schocken classifies his recursive algorithm as O(log n) and
ignores the VM overhead of function calling and local variable
rematerialization.  My schoolbook, long division, approach is constant-time,
O(bitwidth), which is O(1)--assisted with the precomputed power-of-two table.

## Output.jack

The bitmap font is based on the Apple II font and is less thick than the
reference font.

## Screen.jack

Originally I didn't want to do my own Screen routines, but I didn't find
satisfactory existing Jack implementations that didn't suffer from
exceptionally poor performance.  Further, the book's diagonal line drawing
routine only permitted either the x- or y-coordinate to change per step,
resulting in a 45-degree line looking fairly bad.  After some research,
the diagonal special-case became an implemenation of Bresenham's line
algorithm.  As it happens, from my experiments, of various slopped lines,
the reference JackOS is using this as well.

The drawRectangle() was also optimized since my MerlinJack game suffered
a noticable slowness with the repeated drawLine() version.

## Memory.jack
