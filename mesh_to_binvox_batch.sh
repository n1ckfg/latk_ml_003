#!/bin/bash

SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"

cd $DIR

INPUT_DIR=$1
INPUT_EXT=$2
OUTPUT_EXT=$3
DIMS=$4
FILTER=$5

INPUT="$INPUT_DIR/*$INPUT_EXT"
python mesh_to_binvox_batch.py -- $INPUT_DIR $INPUT_EXT $DIMS $FILTER 

