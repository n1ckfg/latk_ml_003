#!/bin/bash

SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"

cd $DIR

DIMS=256


# ~ ~ ~ ~ ~ ~ ~ ~ ~
./mesh_resample.sh "input" "0.5" ".obj" "_resample.ply"
./mesh_to_binvox_batch.sh "input" "_resample.ply" "_pre.ply" "$DIMS" "True" # *_pre.ply -> *.binvox
./binvox_to_h5.sh "input" "$DIMS" # *.binvox -> *.im
# ~ ~ ~ ~ ~ ~ ~ ~ ~

# ~ ~ ~ ~ ~ ~ ~ ~ ~
python test.py --epoch 50 --dataset "input" --img_width "$DIMS" --img_height "$DIMS" --img_depth "$DIMS"
# ~ ~ ~ ~ ~ ~ ~ ~ ~

# ~ ~ ~ ~ ~ ~ ~ ~ ~
./filter_binvox.sh "output" "$DIMS" # *_fake.binvox -> *_fake_filter.binvox
rm output/*fake.binvox
./binvox_to_mesh.sh "output" "$DIMS" # *_fake_filter.binvox -> _post.ply
./Difference_Eigenvalues.sh "output" # *_post.ply -> *_post_edges.ply
./mesh_resample.sh "output" "0.05" "_post_edges.ply" "_resample.ply"
./color_transfer.sh "input" "output" "_resample_fake_filter_post_edges_resample.ply" # -> *final.obj
# ~ ~ ~ ~ ~ ~ ~ ~ ~