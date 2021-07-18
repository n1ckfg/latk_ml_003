import sys, os, glob
import numpy as np
import scipy.ndimage as nd
import h5py
import binvox_rw

def read_h5(path):
    f = h5py.File(path, 'r')
    voxel = f['data'][:]
    f.close()

    return voxel

def write_binvox(data, path, dims=128):
    data = np.rint(data).astype(np.uint8)
    shape = (dims, dims, dims) #data.shape
    translate = [0, 0, 0]
    scale = 1.0
    axis_order = 'xzy'
    v = binvox_rw.Voxels(data, shape, translate, scale, axis_order)

    with open(path, 'bw') as f:
        v.write(f)

def main():
    argv = sys.argv
    argv = argv[argv.index("--") + 1:] # get all args after "--"

    inputPath = argv[0]
    dims = int(argv[1])

    print("Reading from : " + inputPath)
    data = read_h5(inputPath)

    url = ""
    outputPathArray = inputPath.split(".")
    for i in range(0, len(outputPathArray)-1):
        url += outputPathArray[i]
    url += ".binvox"

    print("Writing to: " + url)
    write_binvox(data, url, dims)

main()
