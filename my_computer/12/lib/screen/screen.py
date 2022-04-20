# x: 0 .. 255
# y: 0 .. 511
SCREEN = [x for x in range(0, 512*256 // 2)]
bitmask = [1 << x for x in range(0, 16)]

# fill with checkerboard
for y in range(0, 512):
  for x in range(0, 256 // 2):
    if y % 2 == 0:
      SCREEN[x + y * 256//2] = 0x5555
    else:
      SCREEN[x + y * 256//2] = 0xAAAA

def line(x1, y1, x2, y2):
  tmp = 0

  # normalize
  if x1 > x2:
    tmp = x1
    x1 = x2
    x2 = tmp

    tmp = y1
    y1 = y2
    y2 = tmp

  if y1 == y2:
    print("horizontal")
    print(x1, y1, " to ", x2, y2)

    # line: ( 5, 4) to ( 9, 4)
    # line: (14, 4) to (18, 4)
    #
    #           0        8        16                 32                 48                 64
    #     0:  0 ........|........  ........|........  ........|........  ........|........  .
    #   128:  1 ........|........  ........|........  ........|........  ........|........  .
    #   256:  2 ........|........  ........|........  ........|........  ........|........  .
    #   384:  3 ........|........  ........|........  ........|........  ........|........  .
    #   512:  4 .....xxx|xx....xx  xxxxxxxx|xxxxxxxx  xxxxxxxx|xxxxxxxx  x.......|........  .
    #   640:  5 ........|........  ........|........  ........|........  ........|........  .
    #          15        7      0 15        7      0 15        7      0 15        7      0 15

    # line: (16, 10) to (56, 10)
    #
    #  0        8        16                 32                 48                 64
    #  ........|........  xxxxxxxx|xxxxxxxx  xxxxxxxx|xxxxxxxx  xxxxxxxx|x.......  .
    # 15        7      0 15        7      0 15        7      0 15        7      0 15

    # line: (15, 10) to (63, 10)
    #
    #  0        8        16                 32                 48                 64
    #  ........|.......x  xxxxxxxx|xxxxxxxx  xxxxxxxx|xxxxxxxx  xxxxxxxx|xxxxxxxx  .
    # 15        7      0 15        7      0 15        7      0 15        7      0 15

    # line: (22, 10) to (26, 10)
    #
    #  0        8        16                 32                 48                 64
    #  ........|........  ......xx|xxx.....  ........|........  ........|........  .
    # 15        7      0 15        7      0 15        7      0 15        7      0 15

    # line: (22, 10) to (41, 10)
    #
    #  0        8        16                 32                 48                 64
    #  ........|........  ......xx|xxxxxxxx  xxxxxxxx|xx......  ........|........  .
    # 15        7      0 15        7      0 15        7      0 15        7      0 15

    # line: (5, 10) to (9, 10)
    # line: (16, 10) to (18, 10)
    # line: (46, 10) to (47, 10)
    #
    #  0        8        16                 32                 48                 64
    #  .....xxx|xx......  xxx.....|........  ........|......xx  ........|........  .
    # 15        7      0 15        7      0 15        7      0 15        7      0 15

    start_pixel = x1 % 16
    stop_pixel = x2 % 16

    # determine start/stop offsets
    start_word_offset = x1 // 16    + y1 * 256//2
    stop_word_offset = x2 // 16     + y2 * 256//2

    #start_word_bitmask = bitmask[ 15 - start_pixel ]
    #for bit in range(start_pixel, 15 - start_pixel):
    #  start_word_bitmask = start_word_bitmask | bitmask[bit]

    #stop_word_bitmask = bitmask[ 15 - stop_pixel ]
    #for bit in range(15 - stop_pixel + 1, 15 - stop_pixel):
    #  stop_word_bitmask = stop_word_bitmask | bitmask[bit]

    print("start_pixel    : %d" % start_pixel)
    print("StartWordOffset: %d" % start_word_offset)
    print("stop_pixel     : %d" % stop_pixel)
    print("StopWordOffset : %d" % stop_word_offset)

    if start_word_offset == stop_word_offset:
      bit = 15 - start_pixel
      word_bitmask = 0
      while (bit > 15 - stop_pixel - 1):
        word_bitmask = word_bitmask | bitmask[bit]
        bit = bit - 1

      #word_bitmask = 0
      #for bit in range(15 - start_pixel, 15 - stop_pixel - 1, -1):
      #  word_bitmask = word_bitmask | bitmask[bit]

      print("Before: %04x" % SCREEN[start_word_offset])
      print("StartP: %d" % start_pixel)
      print("StopP : %d" % stop_pixel)
      print("Mask  : %04x (special case: single mask)" % word_bitmask)
      print("After : %04x" % (SCREEN[start_word_offset] | word_bitmask))
    else:
      bit = 15 - start_pixel
      word_bitmask = 0
      while (not (bit < 0)):
        word_bitmask = word_bitmask | bitmask[bit]
        bit = bit - 1

      print("StartM: %04x" % word_bitmask)

      start_word_offset = start_word_offset + 1
      while start_word_offset < stop_word_offset:
        print("BtwenM: ffff")
        start_word_offset = start_word_offset + 1

      bit = 15 - stop_pixel
      word_bitmask = 0
      while (not (15 < bit)):
        word_bitmask = word_bitmask | bitmask[bit]
        bit = bit + 1

      print("StopM : %04x" % word_bitmask)

    return


      #word_bitmask = start_word_bitmask
      #print("Before: %04x" % SCREEN[start_word_offset])
      #print("Mask  : %04x" % word_bitmask)
      #print("After : %04x" % (SCREEN[start_word_offset] | word_bitmask))
      #print(" -- %dx ffff" % (stop_word_offset - start_word_offset))

    #word_bitmask = stop_word_bitmask
    #print("Before: %04x" % SCREEN[stop_word_offset])
    #print("Mask  : %04x" % word_bitmask)
    #print("After : %04x" % (SCREEN[stop_word_offset] | word_bitmask))

    # BEFORE
    # MASK(S)
    # AFTER



  if x1 == x2:
    print("vertical")
    print(x1, y1, " to ", x2, y2)

    # line: ( 3, 1) to ( 3, 5)
    # line: (18, 2) to (18, 4)
    #
    #           0        8        16                 32                 48                 64
    #     0:  0 ........|........  ....x...|........  ........|........  ........|........  .
    #   128:  1 ...x....|........  ....x...|........  ........|........  ........|........  .
    #   256:  2 ...x....|........  ..x.x...|........  ........|........  ........|........  .
    #   384:  3 ...x....|........  ..x.....|........  ........|........  ........|........  .
    #   512:  4 ...x....|........  ..x.....|........  ........|........  ........|........  .
    #   640:  5 ...x....|........  ........|........  ........|........  ........|........  .
    #          15        7      0 15        7      0 15        7      0 15        7      0 15

    bit = 15 - x1 % 16

    # determine start/stop offsets
    word_offset = x1//16 + y1 * 256//2
    stop_word_offset = x2//16  + y2 * 256//2

    word_bitmask = bitmask[bit]
    print("Bit   : %d" % bit)
    print("Mask  : %04x" % word_bitmask)
    print()

    while (word_offset <= stop_word_offset):
      print("Offset: %d" % word_offset)
      print("Before: %04x" % SCREEN[word_offset])
      print("After : %04x" % (SCREEN[word_offset] | word_bitmask))
      print()

      word_offset = word_offset + 256 // 2

  if x2 > x1:
    print("diagonal")

    # line: (0, 0) to (8, 4)
    #
    #    0        8        16                 32                 48                 64
    #  0 x.......|........  ........|........  ........|........  ........|........  .
    #  1 xxx.....|........  ........|........  ........|........  ........|........  .
    #  2 ..xxx...|........  ........|........  ........|........  ........|........  .
    #  3 ....xxx.|........  ........|........  ........|........  ........|........  .
    #  4 ......xx|x.......  ........|........  ........|........  ........|........  .
    #  5 ........|........  ........|........  ........|........  ........|........  .
    #   15        7      0 15        7      0 15        7      0 15        7      0 15

    # line: (0, 0) to (9, 4)
    #
    #    0        8        16                 32                 48                 64
    #  0 x.......|........  ........|........  ........|........  ........|........  .
    #  1 xxxx....|........  ........|........  ........|........  ........|........  .
    #  2 ...xxx..|........  ........|........  ........|........  ........|........  .
    #  3 .....xxx|........  ........|........  ........|........  ........|........  .
    #  4 .......x|xx......  ........|........  ........|........  ........|........  .
    #  5 ........|........  ........|........  ........|........  ........|........  .
    #   15        7      0 15        7      0 15        7      0 15        7      0 15

    # line: (19, 0) to (4, 5)
    #
    #    0        8        16                 32                 48                 64
    #  0 ........|........  xxxx....|........  ........|........  ........|........  .
    #  1 ........|.....xxx  x.......|........  ........|........  ........|........  .
    #  2 ........|..xxxx..  ........|........  ........|........  ........|........  .
    #  3 .......x|xxx.....  ........|........  ........|........  ........|........  .
    #  4 ....xxxx|........  ........|........  ........|........  ........|........  .
    #  5 ....x...|........  ........|........  ........|........  ........|........  .
    #   15        7      0 15        7      0 15        7      0 15        7      0 15

    # line: ( 0, 0) to ( 5, 5)
    # line: (11, 5) to (16, 0)
    # line: (16, 0) to (24, 5)
    # line: (38, 0) to (26, 5)
    #
    #          0        8        16                 32                 48                 64
    #    0:  0 x.......|........  x.......|........  .....xx.|........  ........|........  .
    #  128:  1 .x......|.......x  .xx.....|........  ...xx...|........  ........|........  .
    #  256:  2 ..x.....|......x.  ...xx...|........  xxx.....|........  ........|........  .
    #  384:  3 ...x....|.....x..  .....xx.|......xx  ........|........  ........|........  .
    #  512:  4 ....x...|....x...  .......x|x...xx..  ........|........  ........|........  .
    #  640:  5 .....x..|...x....  ........|x.xx....  ........|........  ........|........  .
    #          15        7      0 15        7      0 15        7      0 15        7      0 15

    #  .....................................................................
    #  ........... (x + dx, y + dy) ........................................
    #  ................@....................................................
    #  ...............@.....................................................
    #  .(overshoot)..@......................................................
    #  ..a++...@....@.......................................................
    #  ....-->.@...@........................................................
    #  .......@...@.........................................................
    #  .......@..@..........................................................
    #  ......@..@...........................................................
    #  .....@..@. m = a/b = dx/dy ..........................................
    #  .....@.@...@@........................................................
    #  .....@@..@@.(undershoot).............................................
    #  ....@@.@@.........^..................................................
    #  ....@@@...........|.b++..............................................
    #  . (x, y) .........|..................................................
    #  .....................................................................

    #start_pixel = x1 % 16
    #stop_pixel = x2 % 16

# Book algorithm:
#    dx = x2 - x1
#    dy = y2 - y1        # can be pos or neg
#
#    if dy > 0:
#        b_step = 1
#        dy_step = dy
#    else:
#        b_step = -1
#        dy_step = -dy
#
#    a = 0
#    b = 0
#
#
#    # a/dx < b/dy :=
#    #       a * dy < b * dx :=
#    #       a * dy - b * dx < 0
#
#    # if a/dx < b/dy then a++ else b++
#    #   ==> if a * dy - b * dx < 0 then a++ else b++
#
#    a_dy_MINUS_b_dx = 0
#
#    # determine start/stop offsets
#    #word_offset = start_pixel + y1 * 256//2
#    #stop_word_offset = stop_pixel  + y2 * 256//2
#
#    # while a <= dx and b <= dy:
#    # while a - dx <= 0 and b - dy <= 0:
#    # while a - dx <= 0 and b - dy <= 0:
#    # while dx - a > 0 or  dy - b > 0:
#    while (dx - a > 0) | (dy - b > 0):
#        pixel = (x1 + a) % 16
#        word_offset = pixel + (y1 + b) * 256//2
#        mask = bitmask[15 - pixel]
#        print("Mask  : %04x    %d,%d" % (bitmask[15 - pixel], x1+a, y1+b))
#
#        if a_dy_MINUS_b_dx < 0:
#            # overshoot
#            a = a + 1
#            a_dy_MINUS_b_dx = a_dy_MINUS_b_dx + dy_step
#        else:
#            # undershoot
#            b = b + b_step
#            a_dy_MINUS_b_dx = a_dy_MINUS_b_dx - dx
#
#    pixel = x2 % 16
#    word_offset = pixel + y2 * 256//2
#    mask = bitmask[15 - pixel]
#    print("Mask  : %04x    %d,%d" % (bitmask[15 - pixel], x2, y2))



# Bresenham's line algorithm
#plotLine(x1, y1, x2, y2)
#    dx = abs(x2 - x1)
#    sx = x1 < x2 ? 1 : -1
#    dy = -abs(y2 - y1)
#    sy = y1 < y2 ? 1 : -1
#    error = dx + dy
#    
#    while true
#        plot(x1, y1)
#        if x1 == x2 && y1 == y2 break
#        e2 = 2 * error
#        if e2 >= dy
#            if x1 == x2 break
#            error = error + dy
#            x1 = x1 + sx
#        end if
#        if e2 <= dx
#            if y1 == y2 break
#            error = error + dx
#            y1 = y1 + sy
#        end if
#    end while


    dx = abs(x2 - x1)
    dy = -abs(y2 - y1)

    if x1 < x2:
        sx = 1
    else:
        sx = -1

    if y1 < y2:
        sy = 1
    else:
        sy = -1

    error = dx + dy

    while True:
        pixel = x1 % 16
        mask = bitmask[15 - pixel]
        word_offset = x1/16 + y1 * 256//2
        print("plot(%d, %d)     mask=%04x @ %d" % (x1, y1, mask, word_offset))

        if x1 == x2 & y1 == y2:
            break;
        e2 = 2 * error
        if e2 >= dy:
            if x1 == x2:
                break
            error = error + dy
            x1 = x1 + sx
        if e2 <= dx:
            if y1 == y2:
                break
            error = error + dx
            y1 = y1 + sy


























def main():
  # HORIZONTAL LINES
  #line(18, 4,    53, 4)
  #line(16, 4,    56, 4)
  #line(15, 4,    63, 4)
  #line(22, 4,    26, 4)       # same word
  #line(22, 4,    41, 4)
  #line( 5, 4,     9, 4)       # same word
  #line(14, 4,     48, 4)
  #line(14, 4,    18, 4)
  #line(46, 4,    47, 4)

  # VERTICAL LINES
  #line(3, 1,    3, 5)
  #line(18, 2,    18, 4)
  #line(20, 0,    20, 3)
  #line(0, 509,    0, 511)

  # DIAGONAL LINES
  #line(0, 0,     8, 4)
  #line(0, 0,     9, 4)
  #line(19, 0,     4, 5)

  # line(0, 0,     5, 5)
  line(11, 5,    16, 0)
  # line(16, 0,    24, 5)
  # line(38, 0,    26, 5)

if __name__ == '__main__':
  main()
