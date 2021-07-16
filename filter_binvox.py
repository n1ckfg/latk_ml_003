# https://github.com/raahii/3dgan-chainer/blob/master/binvox_to_h5.py
# https://github.com/raahii/3dgan-chainer/blob/master/LICENSE
# Convert dataset(.binvox) to numpy array in advance
# to load dataset more efficient.

import sys, os, glob
import numpy as np
import scipy.ndimage as nd
import binvox_rw

argv = sys.argv
argv = argv[argv.index("--") + 1:] # get all args after "--"

inputPath = argv[0]

dims = 128

# filters
dilateReps = 0 #3
erodeReps = 2 #2
gaussianSigma = 0 #0
medianSize = 4 #3
sobelReps = 2 #0
laplaceReps = 1 #0

def read_binvox(path, shape=(dims, dims, dims), fix_coords=True):
    voxel = None
    with open(path, 'rb') as f:
        voxel = binvox_rw.read_as_3d_array(f, fix_coords)
    return voxel

def write_binvox(data, url):
    with open(url, 'wb') as f:
        data.write(f)

def main():
    print("Reading from : " + inputPath)
    bv = read_binvox(inputPath)

    url = ""
    outputPathArray = inputPath.split(".")
    for i in range(0, len(outputPathArray)-1):
        url += outputPathArray[i]
    url += "_filter.binvox"
   
    # filters
    if (dilateReps > 0):
        print("Dilating...")
        for i in range(0, dilateReps):
            nd.binary_dilation(bv.data.copy(), output=bv.data)

    if (sobelReps > 0):
        print ("Sobel filter...")
        for i in range(0, sobelReps):
            nd.sobel(bv.data.copy(), output=bv.data)

    if (gaussianSigma > 0):
        print("Gaussian filter")
        nd.gaussian_filter(bv.data.copy(), sigma=gaussianSigma, output=bv.data)
    
    if (medianSize > 0):
        print("Median filter")
        nd.median_filter(bv.data.copy(), size=medianSize, output=bv.data)
    
    if (laplaceReps > 0):
        print("Laplace filter...")
        for i in range(0, laplaceReps):
            nd.laplace(bv.data.copy(), output=bv.data)

    if (erodeReps > 0):
        print("Eroding...")
        for i in range(0, erodeReps):
            nd.binary_erosion(bv.data.copy(), output=bv.data)

    print("Writing to: " + url)
    write_binvox(bv, url)
    
main()