from cairosvg import svg2png
from xml.dom import minidom
import sys

svg_in = minidom.parse(sys.argv[1])
svg2png(bytestring=svg_in.toprettyxml(), write_to=sys.argv[2])