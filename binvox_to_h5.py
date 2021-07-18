# https://github.com/raahii/3dgan-chainer/blob/master/binvox_to_h5.py
# https://github.com/raahii/3dgan-chainer/blob/master/LICENSE
# Convert dataset(.binvox) to numpy array in advance
# to load dataset more efficient.

import sys, os, glob
import numpy as np
import scipy.ndimage as nd
import h5py
import binvox_rw

def resize(voxel, shape):
    """
    resize voxel shape
    """
    ratio = shape[0] / voxel.shape[0]
    voxel = nd.zoom(voxel,
            ratio,
            order=1, 
            mode='nearest')
    voxel[np.nonzero(voxel)] = 1.0
    return voxel

def read_binvox(path, dims=128, fix_coords=True):
    shape=(dims, dims, dims)

    with open(path, 'rb') as f:
        voxel = binvox_rw.read_as_3d_array(f, fix_coords)
    
    voxel_data = voxel.data.astype(np.float)
    if shape is not None and voxel_data.shape != shape:
        voxel_data = resize(voxel.data.astype(np.float64), shape)

    return voxel_data

# ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
def convert_h5(in_path, out_path, dims=128):
    data = read_binvox(in_path, dims)
    f = h5py.File(out_path, 'w')
    # more compression options: https://docs.h5py.org/en/stable/high/dataset.html
    f.create_dataset('data', data = data, compression='gzip')
    f.flush()
    f.close()

    return out_path

def main():
    argv = sys.argv
    argv = argv[argv.index("--") + 1:] # get all args after "--"

    inputPath = argv[0]
    dims = int(argv[1])

    print("Reading from : " + inputPath)

    url = ""
    outputPathArray = inputPath.split(".")
    for i in range(0, len(outputPathArray)-1):
        url += outputPathArray[i]
    url += ".im"

    print("Writing to: " + url)
    convert_h5(inputPath, url, dims)

main()