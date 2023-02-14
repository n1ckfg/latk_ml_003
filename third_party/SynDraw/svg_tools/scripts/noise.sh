#!/bin/sh
SVG_DISTURBER=../../../svg_tools/svg_disturber.py
SVG_RESAMPLER=../../../svg_tools/svg_resampler.py
SVG_RASTERIZER=../../../svg_tools/svg_rasterizer.py

SVG_IN=$1
PNG_OUT=$2

# resampling svg so that average line length is 10
python $SVG_RESAMPLER $SVG_IN temp 10 0.8 1.2

# adding random noise : 5 per point, 10 maximum overstroking, potential oversketching
python $SVG_DISTURBER temp $PNG_OUT -n 5 -c -os 10 -min 1 -max 1 -bg

# convert to png
#python3 $SVG_RASTERIZER temp2 $PNG_OUT

# remove temporary
# rm -rf temp temp2