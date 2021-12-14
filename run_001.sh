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

./pipeline_004.sh "$INPUT_DIR" "$FIRST_INPUT_EXT" "$OUTPUT_DIR" "$DIMS" "$EPOCH" "$RESAMPLE"

INPUT_DIR="output"
NUM_CLUSTERS=10
DIMS=3
NUM_STROKES=1000
MIN_STROKE_SIZE=4
MAX_STROKE_SIZE=9999

python latk_process_006.py -- "$INPUT_DIR" "_final.ply" "$NUM_CLUSTERS" "$DIMS" "$NUM_STROKES" "$MIN_STROKE_SIZE" "$MAX_STROKE_SIZE"



