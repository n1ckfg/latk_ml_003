import sys
import os
import pymeshlab as ml

def changeExtension(_url, _newExt):
    returns = ""
    returnsPathArray = _url.split(".")
    for i in range(0, len(returnsPathArray)-1):
        returns += returnsPathArray[i]
    returns += _newExt
    return returns

def main():
    argv = sys.argv
    argv = argv[argv.index("--") + 1:] # get all args after "--"

    inputPath = argv[0]
    samplePercentage = float(argv[1])
    outputFormat = argv[2]

    outputPath = changeExtension(inputPath, outputFormat)

    ms = ml.MeshSet()
    ms.load_new_mesh(inputPath)

    newSampleNum = int(ms.current_mesh().vertex_number() * samplePercentage)
    if (newSampleNum < 1):
        newSampleNum = 1

    ms.apply_filter("poisson_disk_sampling", samplenum=newSampleNum, subsample=True)
    ms.save_current_mesh(outputPath)

main()