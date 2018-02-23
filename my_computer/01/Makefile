# Hardware simulator checks

all: and or not xor mux dmux not16 and16 or16 mux16 or8way mux4way16 mux8way16 dmux4way dmux8way

dmux8way:
	n2ths DMux8Way.tst
dmux4way:
	n2ths DMux4Way.tst
mux8way16:
	n2ths Mux8Way16.tst
mux4way16:
	n2ths Mux4Way16.tst
or8way:
	n2ths Or8way.tst
mux16:
	n2ths Mux16.tst
or16:
	n2ths Or16.tst
and16:
	n2ths And16.tst
not16:
	n2ths Not16.tst
dmux:
	n2ths DMux.tst
mux:
	n2ths Mux.tst
xor:
	n2ths Xor.tst
and:
	n2ths And.tst
or:
	n2ths Or.tst
not:
	n2ths Not.tst

clean:
	rm -f And.out  And16.out  DMux.out  DMux4Way.out  DMux8Way.out  Mux.out  Mux16.out  Mux4Way16.out  Mux8Way16.out  Not.out  Not16.out  Or.out  Or16.out  Or8Way.out  Xor.out
