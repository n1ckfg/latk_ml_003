INPUT_DIR="input"
OUTPUT_DIR="output"
DIMS=256
EPOCH=100
RESAMPLE=0.1

./pipeline004.sh "$INPUT_DIR" "$OUTPUT_DIR" "$DIMS" "$EPOCH" "$RESAMPLE"

INPUT_DIR="output"
DIMS=3

python skel_to_latk.py -- "$INPUT_DIR" "_final.ply" "$DIMS" "False"


