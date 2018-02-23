# Hardware simulator checks

all: halfadd fulladd add16 inc16 aluone alu

halfadd:
	n2ths HalfAdder.tst

fulladd:
	n2ths FullAdder.tst

add16:
	n2ths Add16.tst

inc16:
	n2ths Inc16.tst

aluone:
	n2ths ALU-nostat.tst

alu:
	n2ths ALU.tst

clean:
	rm -f ALU-nostat.out  ALU.out  Add16.out  FullAdder.out  HalfAdder.out  Inc16.out  ZeroCheck.out
