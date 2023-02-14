from cairosvg import svg2png
from PIL import Image
import sys

svg2png(url=sys.argv[1], write_to='temp.png')
im1 = Image.open(sys.argv[2])
im2 = Image.open('temp.png')
im1.paste(im2, mask=im2)
im1.save(sys.argv[3])