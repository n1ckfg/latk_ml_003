#!/bin/bash

SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"

cd $DIR

SOURCE_DIR=$1
DEST_DIR=$2
DEST_EXT=$3

for INPUT in "$DIR/$SOURCE_DIR"/*_pre.ply
do
  echo "Restoring vertex color $INPUT"
  python color_transfer.py -- "$INPUT" "$DIR"/"$DEST_DIR" "$DEST_EXT"
done
