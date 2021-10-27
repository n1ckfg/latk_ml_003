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

# ~ ~ ~ ~ ~ ~ ~ ~ ~
echo "1. Preprocessing..."
echo "1.1. Resample point clouds."
python mesh_resample.py -- "$INPUT_DIR" "$RESAMPLE" ".obj" "_resample.ply"

echo "1.2. Convert point clouds to voxel grids."
python mesh_to_binvox.py -- "$INPUT_DIR" "_resample.ply" "_pre.ply" "$DIMS" "True"  # *_pre.ply -> *.binvox
python binvox_to_h5.py -- "$INPUT_DIR" "$DIMS" # *.binvox -> *.im

# ~ ~ ~ ~ ~ ~ ~ ~ ~
echo "2. Inference..."
python test.py --epoch "$EPOCH" --dataset "$INPUT_DIR" --img_width "$DIMS" --img_height "$DIMS" --img_depth "$DIMS"

# ~ ~ ~ ~ ~ ~ ~ ~ ~
echo "3. Postprocessing..."
echo "3.1. Filter output."
python filter_binvox.py -- "$OUTPUT_DIR" "$DIMS" # *_fake.binvox -> *_fake_filter.binvox
rm output/*fake.binvox

echo "3.2. Convert voxel grids to point clouds."
python binvox_to_mesh.py -- "$OUTPUT_DIR" "$DIMS" # *_fake_filter.binvox -> _post.ply

echo "3.3. Find edges in point clouds."
python Difference_Eigenvalues.py -- "$OUTPUT_DIR" # *_post.ply -> *_post_edges.ply

echo "3.4. Transfer vertex color."
python color_transfer.py -- "$INPUT_DIR" "$OUTPUT_DIR" "_resample_fake_filter_post_edges.ply" # -> *final.obj
