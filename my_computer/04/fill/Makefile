all: Fill.hack FillAutomatic.out

Fill.hack: Fill.asm
	n2tas Fill.asm

FillAutomatic.out: FillAutomatic.tst Fill.asm
	n2tce FillAutomatic.tst

clean:
	rm -f Fill.hack FillAutomatic.out
