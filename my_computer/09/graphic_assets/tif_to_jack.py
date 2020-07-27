from PIL import Image
import numpy as np
import math

# load the image (black and white TIFF files)
#image = Image.open('merlin_black_pip.tif')
#image = Image.open('goal.tif')
#image = Image.open('goal-1.tif')
#image = Image.open('goal-2.tif')
#image = Image.open('goal-3.tif')
image = Image.open('merlin_grid_numbers.tif')
#image = Image.open('merlin_title.tif')
#image = Image.open('merlin_title2.tif')

# valid TIFF image info
#   image.format_description
#   'Adobe TIFF'
#
#   image.info
#   {'compression': 'raw', 'dpi': (72, 72)}
#

# convert image to numpy bit array and invert pixel
pixels = np.asarray(image, dtype=np.uint8) ^ 1

# Top "0" image of merlin_grid_numbers.tif:
# pixels[:14]                                                                                   │ 46 # create Jack code
# array([[0, 0, 1, 1, 1, 1, 1, 1, 0, 0],
#        [0, 0, 1, 1, 1, 1, 1, 1, 0, 0],
#        [1, 1, 0, 0, 0, 0, 0, 0, 1, 1],
#        [1, 1, 0, 0, 0, 0, 0, 0, 1, 1],
#        [1, 1, 0, 0, 0, 0, 1, 1, 1, 1],
#        [1, 1, 0, 0, 0, 0, 1, 1, 1, 1],
#        [1, 1, 0, 0, 1, 1, 0, 0, 1, 1],
#        [1, 1, 0, 0, 1, 1, 0, 0, 1, 1],
#        [1, 1, 1, 1, 0, 0, 0, 0, 1, 1],
#        [1, 1, 1, 1, 0, 0, 0, 0, 1, 1],
#        [1, 1, 0, 0, 0, 0, 0, 0, 1, 1],
#        [1, 1, 0, 0, 0, 0, 0, 0, 1, 1],
#        [0, 0, 1, 1, 1, 1, 1, 1, 0, 0],
#        [0, 0, 1, 1, 1, 1, 1, 1, 0, 0]], dtype=uint8)

y_dim = pixels.shape[0]
x_dim = pixels.shape[1]

x_dim_padded = math.ceil(x_dim / 8) * 8

# pad out the pixel array
if x_dim_padded != pixels.shape[0]:
  d = x_dim_padded - x_dim
  a = list([np.append(r, np.array(d * [0], dtype=np.uint8)) for r in pixels])
  pixels_padded = np.array(a)
else:
  pixels_padded = pixels

# flip all the pixels along vertical axis
pixels_padded_rev = np.flip(pixels_padded, 1)

# convert numpy bit array to int8/uint16/int16
#pixels_int8 = np.packbits(pixels_padded_rev, axis=1).view(np.uint8)
#pixels_uint16 = np.packbits(pixels_padded_rev, axis=1).view(np.uint16)
pixels_int16 = np.packbits(pixels_padded_rev, axis=1).view(np.int16)

# create Jack code
# Hack screen is 256h x 512w pixels
# row - vertical pixel offset
# dblcol - horizontal 16-pixel offset (32, 16-pixel dblcols per row)
num_rows = pixels_int16.shape[0]
num_dblcols = pixels_int16.shape[1]

print("class Image")
print("{")
print("  function int get_x_size() { return %d; }" % (num_dblcols*16))
print("  function int get_y_size() { return %d; }" % (num_rows))
print()

print("function void draw(int dblcol, int row)")
print("{")
print("  var int base_address;")
print("  let base_address = 16384+(32*row)+dblcol;")
print()
row_offset = 0
for row in pixels_int16.byteswap():
  if num_dblcols > 1: print("  // %s" % (70*'-'))
  last_col = len(row) - 1
  for col,val in enumerate(row):
    # Work around bug in reference jack compiler where -32768 is reported
    # as "too big"
    if val == 0 - 0x8000:
      print("  do Memory.poke(base_address+%2d+%4d,32767+1);" %
            (last_col - col, row_offset))
    else:
      if val == 0:
        pass
      else:
        print("  %sdo Memory.poke(base_address+%2d+%4d, %6d);" %
              ("// " if val == 0 else "",
                last_col - col, row_offset, val))
  row_offset = row_offset+32

print("\n  return;");
print("}\n");

print("function void blank(int dblcol, int row)")
print("{")

print("  var int x1;")
print("  var int y1;")
print("  var int x2;")
print("  var int y2;")
print("  let x1 = dblcol*16;")
print("  let y1 = row;")
print("  let x2 = x1+%d;" % (num_dblcols*16 - 1));
print("  let y2 = y1+%d;" % (num_rows))

print()

print("  do Screen.setColor(false);")
print("  do Screen.drawRectangle(x1, y1, x2, y2);")

print("\n  return;")
print("}")
print("}")
