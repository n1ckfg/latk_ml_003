@echo off

cd %~dp0

set INPUT_DIR=input
set FIRST_INPUT_EXT=.obj
set OUTPUT_DIR=output
set DIMS=256
set EPOCH=100
set RESAMPLE=0.5

del %INPUT_DIR%\*.binvox
del %INPUT_DIR%\*.im
del %INPUT_DIR%\*resample.ply
del %INPUT_DIR%\*resample.obj
del %INPUT_DIR%\*pre.ply
del %INPUT_DIR%\*pre.obj
del %OUTPUT_DIR%\*.binvox
del %OUTPUT_DIR%\*.im
del %OUTPUT_DIR%\*.ply
del %OUTPUT_DIR%\*.obj

call pipeline_004.bat %INPUT_DIR% %FIRST_INPUT_EXT% %OUTPUT_DIR% %DIMS% %EPOCH% %RESAMPLE%

set INPUT_DIR=output
set NUM_CLUSTERS=10
set DIMS=3
set NUM_STROKES=1000
set MIN_STROKE_SIZE=4
set MAX_STROKE_SIZE=9999

python latk_process_006.py -- %INPUT_DIR% _final.ply %NUM_CLUSTERS% %DIMS% %NUM_STROKES% %MIN_STROKE_SIZE% %MAX_STROKE_SIZE%

@pause

