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
OUTPUT_DIR=$2
DIMS=$3
EPOCH=$4
RESAMPLE=$5

echo "1. Resample point clouds."
python mesh_resample.py -- "$INPUT_DIR" "$RESAMPLE" ".obj" "_resample.ply"

echo "2. Convert point clouds to voxel grids."
python mesh_to_binvox.py -- "$INPUT_DIR" "_resample.ply" "_pre.ply" "$DIMS" "True"  # *_pre.ply -> *.binvox

echo "3. Convert voxel grids to point clouds."
python binvox_to_mesh.py -- "$INPUT_DIR" ".binvox" "$DIMS" # *_fake_filter.binvox -> _post.ply

echo "4. Find edges in point clouds."
python Difference_Eigenvalues.py -- "$INPUT_DIR" # *_post.ply -> *_post_edges.ply

