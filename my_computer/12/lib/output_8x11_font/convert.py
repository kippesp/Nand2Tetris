import sys


def reverse_bits(num):
  NO_OF_BITS = 8
  reverse_num = 0

  for i in range(0, NO_OF_BITS):
    if num & (1 << i):
      reverse_num |= 1 << ((NO_OF_BITS - 1) - i)
  return reverse_num

def make_create(val, mask):

  outstr = "do Output.create("
  outstr += "%i," % val
  outstr += ",".join(["%i" % v for v in mask])
  outstr += ");"

  outstr = "        %-62s" % outstr
  outstr += "// "
  outstr += chr(val)

  print(outstr)

def make_char(lines):
  ascii_val = int(lines[0][11:].split(',')[0])

  mask_values = []

  for raw_line in lines[2:-1]:
    mask_line = raw_line[4:12]
    mask_bits = 0
    for ch in mask_line:
      mask_bits = mask_bits << 1
      if ch == '@':
        mask_bits = mask_bits | 0x1

    mask_values.append(reverse_bits(mask_bits))

  make_create(ascii_val, mask_values)


char_repr = []

with open("8x11.txt") as fp:

  for line in fp.readlines():
    line = line.strip()

    if line:
      char_repr.append(line)
    else:
      make_char(char_repr)
      char_repr = []
