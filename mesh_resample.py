import sys
import os
import pymeshlab as ml
from pathlib import Path

def main():
    argv = sys.argv
    argv = argv[argv.index("--") + 1:] # get all args after "--"

    inputPath = argv[0]
    samplePercentage = float(argv[1])
    outputFormat = argv[2].lower()

    outputPath = Path(inputPath).stem + "_resample." + outputFormat

    ms = ml.MeshSet()
    ms.load_new_mesh(inputPath)

    newSampleNum = int(ms.current_mesh().vertex_number() * samplePercentage)
    if (newSampleNum < 1):
        newSampleNum = 1

    ms.apply_filter("poisson_disk_sampling", samplenum=newSampleNum, subsample=True)
    ms.save_current_mesh(outputPath)

main()