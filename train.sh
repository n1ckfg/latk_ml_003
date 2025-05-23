DIMS=128
START_EPOCH=0
TOTAL_EPOCHS=101
BATCH_SIZE=10
NUM_CPUS=8
CHECKPOINT_INTERVAL=10

#some quirks:
#* You must set a checkpoint interval or the model won't be saved.
#* The exit condition is off by one, so add an extra epoch.
#* The default 8 threads is probably too high for free tier Colab, but okay for the paid tiers.

python vox2vox/train.py --epoch "$START_EPOCH" --dataset "input" --img_width "$DIMS" --img_height "$DIMS" --img_depth "$DIMS" --batch_size "$BATCH_SIZE" --n_cpu "$NUM_CPUS" --checkpoint_interval "$CHECKPOINT_INTERVAL" --n_epochs "$TOTAL_EPOCHS"

#python train.py --dataset "input" --n_cpu 8 --checkpoint_interval 50 --n_epochs 201
