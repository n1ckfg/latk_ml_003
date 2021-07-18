import sys
import trimesh
import binvox_rw
import numpy as np
import scipy.ndimage as nd

# https://stackoverflow.com/questions/19299155/normalize-a-vector-of-3d-coordinates-to-be-in-between-0-and-1/19301193
def scale_numpy_array(arr, min_v, max_v):
    new_range = (min_v, max_v)
    max_range = max(new_range)
    min_range = min(new_range)
    scaled_unit = (max_range - min_range) / (np.max(arr) - np.min(arr))
    return arr*scaled_unit - np.min(arr)*scaled_unit + min_range

def meshToBinvox(url, dim=128, axis='xyz'):
    dims = (dim, dim, dim)
    data = np.zeros((dims[0], dims[1], dims[2]), dtype=bool)
    translate = (0, 0, 0)
    scale = 1
    axis_order = axis
    bv = binvox_rw.Voxels(data, dims, translate, scale, axis_order)

    mesh = trimesh.load(url)
    verts = scale_numpy_array(mesh.vertices, 0, dim-1)

    for vert in verts:
        x = int(vert[0])
        y = int(vert[1])
        z = int(vert[2])
        data[x][y][z] = True

    for i in range(0, 1): # 1
        nd.binary_dilation(bv.data.copy(), output=bv.data)

    for i in range(0, 3): # 3
        nd.sobel(bv.data.copy(), output=bv.data)

    nd.median_filter(bv.data.copy(), size=4, output=bv.data) # 4

    for i in range(0, 2): # 2
        nd.laplace(bv.data.copy(), output=bv.data)

    for i in range(0, 0): # 0
        nd.binary_erosion(bv.data.copy(), output=bv.data)

    outputUrl = ""
    outputPathArray = url.split(".")
    for i in range(0, len(outputPathArray)-1):
        outputUrl += outputPathArray[i]
    outputUrl += ".binvox"

    with open(outputUrl, 'wb') as f:
        bv.write(f)

def main():
    argv = sys.argv
    argv = argv[argv.index("--") + 1:] # get all args after "--"

    inputPath = argv[0]
    meshToBinvox(inputPath)

main()