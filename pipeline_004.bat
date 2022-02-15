@echo off

cd %~dp0

set INPUT_DIR=%1
set FIRST_INPUT_EXT=%2
set OUTPUT_DIR=%3
set DIMS=%4
set EPOCH=%5
set RESAMPLE=%6

rem ~ ~ ~ ~ ~ ~ ~ ~ ~
echo
echo 1. Preprocessing...
echo
echo 1.1. Resample point clouds.
python mesh_resample.py -- %INPUT_DIR% %RESAMPLE% %FIRST_INPUT_EXT% _resample.ply

echo
echo 1.2. Convert point clouds to voxel grids.
python mesh_to_binvox.py -- %INPUT_DIR% _resample.ply _pre.ply %DIMS% True  
rem *_pre.ply -> *.binvox

echo 
echo 1.3. Convert binvox to hdf5.
python binvox_to_h5.py -- %INPUT_DIR% %DIMS% 
rem *.binvox -> *.im

rem ~ ~ ~ ~ ~ ~ ~ ~ ~
echo
echo 2. Inference...
python test.py --epoch %EPOCH% --dataset %INPUT_DIR% --img_width %DIMS% --img_height %DIMS% --img_depth %DIMS%

rem ~ ~ ~ ~ ~ ~ ~ ~ ~
echo
echo 3. Postprocessing...
echo
echo 3.1. Convert voxel grids to point clouds.
python binvox_to_mesh.py -- %OUTPUT_DIR% fake.binvox %DIMS% 
rem *_fake_filter.binvox -> _post.ply

echo
echo 3.2. Transfer vertex color.
python color_transfer.py -- %INPUT_DIR% _resample_pre.ply %OUTPUT_DIR% _resample_fake_post.ply _final.ply

@pause