#!/bin/bash

SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"

cd $DIR

dos2unix *.sh
dos2unix test.template
chmod +x *.sh

INPUT_DIR="input"
FIRST_INPUT_EXT=".obj"
OUTPUT_DIR="output"
DIMS=256
EPOCH=100
RESAMPLE=0.5

rm $INPUT_DIR/*.binvox
rm $INPUT_DIR/*.im
rm $INPUT_DIR/*resample.ply
rm $INPUT_DIR/*resample.obj
rm $INPUT_DIR/*pre.ply
rm $INPUT_DIR/*pre.obj
rm $OUTPUT_DIR/*.binvox
rm $OUTPUT_DIR/*.im
rm $OUTPUT_DIR/*.ply
rm $OUTPUT_DIR/*.obj

./pipeline.sh "$INPUT_DIR" "$FIRST_INPUT_EXT" "$OUTPUT_DIR" "$DIMS" "$EPOCH" "$RESAMPLE"

INPUT_DIR="output"
INPUT_EXT="final.ply"
RESAMPLE=0.5
MIN_POINTS=3
SEARCH_DIST=0.02

python latk_process.py -- "$INPUT_DIR" "$INPUT_EXT" "$RESAMPLE" "$MIN_POINTS" "$SEARCH_DIST"




