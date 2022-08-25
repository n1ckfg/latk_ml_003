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
FIRST_INPUT_EXT=$2
OUTPUT_DIR=$3
DIMS=$4
EPOCH=$5
RESAMPLE=$6

# ~ ~ ~ ~ ~ ~ ~ ~ ~
echo
echo "1. Preprocessing..."
echo
echo "1.1. Resample point clouds."
python mesh_resample.py -- "$INPUT_DIR" "$RESAMPLE" "$FIRST_INPUT_EXT" "_resample.ply"

echo
echo "1.2. Convert point clouds to voxel grids."
python mesh_to_binvox.py -- "$INPUT_DIR" "_resample.ply" "_pre.ply" "$DIMS" "True"  # *_pre.ply -> *.binvox

echo 
echo "1.3. Convert binvox to hdf5."
python binvox_to_h5.py -- "$INPUT_DIR" "$DIMS" # *.binvox -> *.im

# ~ ~ ~ ~ ~ ~ ~ ~ ~
echo
echo "2. Inference..."
python test.py --epoch "$EPOCH" --dataset "$INPUT_DIR" --img_width "$DIMS" --img_height "$DIMS" --img_depth "$DIMS"

# ~ ~ ~ ~ ~ ~ ~ ~ ~
echo
echo "3. Postprocessing..."
echo
echo "3.1. Convert voxel grids to point clouds."
python binvox_to_mesh.py -- "$OUTPUT_DIR" "fake.binvox" "$DIMS" # *_fake_filter.binvox -> _post.ply

#echo
#echo "3.2. Find edges in point clouds."
#python Difference_Eigenvalues.py -- "$OUTPUT_DIR" # *_post.ply -> *_post_edges.ply

#echo
#echo "3.3. Transfer vertex color."
#python color_transfer.py -- "$INPUT_DIR" "_resample_pre.ply" "$OUTPUT_DIR" "_resample_fake_post_edges.ply" "_final.ply"

echo
echo "3.2. Transfer vertex color."
python color_transfer.py -- "$INPUT_DIR" "_resample_pre.ply" "$OUTPUT_DIR" "_resample_fake_post.ply" "_final.ply"

