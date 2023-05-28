# does not include the properties files preparation

SYNDRAW_EXE=../SynDraw/build/SynDraw
NORMALS_EXE=../SynDraw/build/SynDraw_Normal
RESAMPLER_PY=../svg_tools/svg_resampler.py 
DISTURBER_PY=../svg_tools/svg_disturber.py
RASTERIZER_PY=../svg_tools/svg_rasterizer.py

OFF_DIR=./off/
SVG_DIR=./svg/
PNG_DIR=./png/
TMP_DIR=./tmp/
NML_DIR=./normals/
PPT_DIR=./properties/

N=20000
V=8

for i in `seq 0 $N`; do
	for j in `seq 0 $V`:; do
		props=${PPT_DIR}${i}_${j}.properties
		norml=${NML_DIR}${i}_${j}.png
		svg_0=${SVG_DIR}${i}_${j}.svg
		svg_1=${TMP_DIR}${i}_${j}_resmp.svg
		svg_2=${TMP_DIR}${i}_${j}_noisy.svg
		png=${PNG_DIR}${i}_${j}.png

		cmd1="$SYNDRAW_EXE -p $props"
		cmd2="$NORMALS_EXE -p $props -o $norml -m 2 -b 1"
		cmd3="python $RESAMPLER_PY $svg_0 $svg_1 2 1.8 2.2"
		cmd4="python $DISTURBER_PY $svg_1 $svg_2 -pen 1.5 -penv 0.5 -n 0.7 -max 2 -os 4 -c -u -f"
		cmd5="python $RASTERIZER_PY $svg_2 $png"

		echo -e "$cmd1 &&\n $cmd2 &&\n $cmd3 &&\n $cmd4 &&\n $cmd5 &"
		$cmd1 && $cmd2 && $cmd3 && $cmd4 && $cmd5

	done

	# [[ $((i)) -ne 0 ]] && [[ $((i % 10)) -eq 0 ]] && wait
done

