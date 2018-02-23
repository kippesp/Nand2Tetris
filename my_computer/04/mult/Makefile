all: Mult.hack Mult.out

Mult.hack: Mult.asm
	n2tas Mult.asm

Mult.out: Mult.tst Mult.asm
	n2tce Mult.tst

clean:
	rm -f Mult.hack Mult.out
