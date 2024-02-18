@echo off

cd %~dp0

set INPUT_DIR=input
set FIRST_INPUT_EXT=.ply
set OUTPUT_DIR=output
set DIMS=256
set EPOCH=100
set RESAMPLE=0.5

rem del %INPUT_DIR%\*.binvox
rem del %INPUT_DIR%\*.im
rem del %INPUT_DIR%\*resample.ply
rem del %INPUT_DIR%\*resample.obj
rem del %INPUT_DIR%\*pre.ply
rem del %INPUT_DIR%\*pre.obj
rem del %OUTPUT_DIR%\*.binvox
rem del %OUTPUT_DIR%\*.im
rem del %OUTPUT_DIR%\*.ply
rem del %OUTPUT_DIR%\*.obj

rem call pipeline_004.bat %INPUT_DIR% %FIRST_INPUT_EXT% %OUTPUT_DIR% %DIMS% %EPOCH% %RESAMPLE%

set INPUT_DIR=output
set INPUT_EXT=final.ply
set RESAMPLE=0.1
set MIN_POINTS=2
set NUM_CLUSTERS=10
set DIMS=3
set NUM_STROKES=1000
set MIN_STROKE_SIZE=4
set MAX_STROKE_SIZE=9999
set MAX_SIMILARITY=0.5
set DO_SKELETON=0

rem python latk_process_001.py -- %INPUT_DIR% %INPUT_EXT% %DIMS% %DO_SKELETON%
rem python latk_process_002.py -- %INPUT_DIR% %INPUT_EXT% %DIMS% %NUM_STROKES% %MIN_STROKE_SIZE% %MAX_STROKE_SIZE% %MAX_SIMILARITY%
rem python latk_process_003.py -- %INPUT_DIR% %INPUT_EXT% %DIMS% %NUM_STROKES% %MIN_STROKE_SIZE% %MAX_STROKE_SIZE% %MAX_SIMILARITY%
rem python latk_process_004.py -- %INPUT_DIR% %INPUT_EXT% %NUM_CLUSTERS% %DIMS% %NUM_STROKES% %MIN_STROKE_SIZE% %MAX_STROKE_SIZE% %MAX_SIMILARITY%
rem python latk_process_005.py -- %INPUT_DIR% %INPUT_EXT% %NUM_CLUSTERS% %DIMS% %NUM_STROKES% %MIN_STROKE_SIZE% %MAX_STROKE_SIZE% %MAX_SIMILARITY%
python latk_process_006.py -- %INPUT_DIR% %INPUT_EXT% %NUM_CLUSTERS% %DIMS% %NUM_STROKES% %MIN_STROKE_SIZE% %MAX_STROKE_SIZE%
rem python latk_process_007.py -- %INPUT_DIR% %INPUT_EXT% %RESAMPLE% %MIN_POINTS%

@pause


