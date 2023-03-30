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

call pipeline.bat %INPUT_DIR% %FIRST_INPUT_EXT% %OUTPUT_DIR% %DIMS% %EPOCH% %RESAMPLE%

set INPUT_DIR=output
set INPUT_EXT=final.ply
set RESAMPLE=0.25
set MIN_POINTS=10

python latk_process.py -- %INPUT_DIR% %INPUT_EXT% %RESAMPLE% %MIN_POINTS%

@pause


