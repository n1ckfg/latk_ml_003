import numpy as np
from PIL import Image
import sys


A = np.array(Image.open(sys.argv[1]).convert('RGB'))
B = np.array(Image.open(sys.argv[2]).convert("RGB"))

pix = np.hstack( np.asarray(i) for i in [A, B] )
img = Image.fromarray(pix)

img.save(sys.argv[3])
