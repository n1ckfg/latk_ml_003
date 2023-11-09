@echo off

set DIMS=128
set START_EPOCH=0
set TOTAL_EPOCHS=101
set BATCH_SIZE=10
set NUM_CPUS=8
set CHECKPOINT_INTERVAL=10

rem some quirks:
rem * You must set a checkpoint interval or the model won't be saved.
rem * The exit condition is off by one, so add an extra epoch.
rem * The default 8 threads is probably too high for free tier Colab, but okay for the paid tiers.

python vox2vox\train.py --epoch %START_EPOCH% --dataset input --img_width %DIMS% --img_height %DIMS% --img_depth %DIMS% --batch_size %BATCH_SIZE% --n_cpu %NUM_CPUS% --checkpoint_interval %CHECKPOINT_INTERVAL% --n_epochs %TOTAL_EPOCHS%

@pause

