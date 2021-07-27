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

def binvoxToMesh(url, dims=128, axis='xyz'):
    voxel = None
    with open(url, 'rb') as f:
        voxel = binvox_rw.read_as_3d_array(f, True) # fix coords
    verts = []
    for x in range(0, dims):
        for y in range(0, dims):
            for z in range(0, dims):
                if (voxel.data[x][y][z] == True):
                    verts.append([dims-1-z, y, x])
    mesh = trimesh.Trimesh(vertices=verts, process=False)
    newMeshUrl = changeExtension(url, "ply", "_post")
    mesh.export(newMeshUrl)

def meshToBinvox(url, dims=128, doFilter=False, axis='xyz'):
    shape = (dims, dims, dims)
    data = np.zeros(shape, dtype=bool)
    translate = (0, 0, 0)
    scale = 1
    axis_order = axis
    bv = binvox_rw.Voxels(data, shape, translate, scale, axis_order)

    mesh = trimesh.load(url)
    verts = scale_numpy_array(mesh.vertices, 0, dims-1)
    mesh.vertices = verts
    newMeshUrl = changeExtension(url, "ply", "_pre")
    mesh.export(newMeshUrl)

    for vert in verts:
        x = dims - 1 - int(vert[0])
        y = int(vert[1])
        z = int(vert[2])
        data[x][y][z] = True

    if (doFilter == True):
        for i in range(0, 1): # 1
            nd.binary_dilation(bv.data.copy(), output=bv.data)

        for i in range(0, 3): # 3
            nd.sobel(bv.data.copy(), output=bv.data)

        nd.median_filter(bv.data.copy(), size=4, output=bv.data) # 4

        for i in range(0, 2): # 2
            nd.laplace(bv.data.copy(), output=bv.data)

        for i in range(0, 0): # 0
            nd.binary_erosion(bv.data.copy(), output=bv.data)

    outputUrl = changeExtension(url, "binvox")

    with open(outputUrl, 'wb') as f:
        bv.write(f)

def changeExtension(_url, _newExt, _append=""):
    returns = ""
    returnsPathArray = _url.split(".")
    for i in range(0, len(returnsPathArray)-1):
        returns += returnsPathArray[i]
    returns += _append + "." + _newExt
    return returns

