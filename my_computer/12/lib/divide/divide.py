bitmask = [2**idx for idx in range(0,16)]

# quotient = dividend / divisor

# dividend = 0000_0100_1111_0001 (0x04f1)
# divisor = 1101 (0x0d)
# quotient = 1100_0001 (0x61)

dividend = 0x04f1
divisor = 0x0d

#dividend = 0x62fe
#divisor = 0x01e8

#dividend = 0x01e8
#divisor = 0x62fe

quotient = 0
partial_dividend = 0
for idx in range(15, -1, -1):
  if dividend & bitmask[idx]:
    partial_dividend = partial_dividend + partial_dividend + 1
  else:
    partial_dividend = partial_dividend + partial_dividend

  if divisor <= partial_dividend:
    partial_dividend = partial_dividend - divisor
    quotient = quotient + quotient + 1
  else:
    quotient = quotient + quotient + 0

print("%04x" % quotient)
remainder = partial_dividend
print("%04x" % remainder)

print("====")

print("%04x" % int(dividend / divisor))
print("%04x" % (dividend % divisor))
