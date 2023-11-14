DIMS=256
START_EPOCH=0
TOTAL_EPOCHS=101
BATCH_SIZE=2
NUM_CPUS=8
CHECKPOINT_INTERVAL=10

#some quirks:
#* You must set a checkpoint interval or the model won't be saved.
#* The exit condition is off by one, so add an extra epoch.
#* The default 8 threads is probably too high for free tier Colab, but okay for the paid tiers.

python vox2vox/train.py --epoch "$START_EPOCH" --dataset "input256" --img_width "$DIMS" --img_height "$DIMS" --img_depth "$DIMS" --batch_size "$BATCH_SIZE" --n_cpu "$NUM_CPUS" --checkpoint_interval "$CHECKPOINT_INTERVAL" --n_epochs "$TOTAL_EPOCHS"

