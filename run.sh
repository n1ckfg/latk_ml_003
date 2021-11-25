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

INPUT_DIR="input"
FIRST_INPUT_EXT=".obj"
OUTPUT_DIR="output"
DIMS=256
EPOCH=100
RESAMPLE=0.1

./pipeline004.sh "$INPUT_DIR" "$FIRST_INPUT_EXT" "$OUTPUT_DIR" "$DIMS" "$EPOCH" "$RESAMPLE"

INPUT_DIR="output"
DIMS=3
NUM_STROKES=400
MIN_STROKE_SIZE=5
MAX_STROKE_SIZE=200
MAX_SIMILARITY=0.9

python paths_to_latk.py -- "$INPUT_DIR" "_final.ply" "$DIMS" "$NUM_STROKES" "$MIN_STROKE_SIZE" "$MAX_STROKE_SIZE" "$MAX_SIMILARITY"



