import sys
import os
import numpy as np
from skeletonize import skeletonize
import latk
import binvox_rw
from random import uniform as rnd
import pymeshlab as ml

def meshToVoxels(vertices=None, _dims=128): #, axis='xyz'):
	shape = (_dims, _dims, _dims)
    data = np.zeros(shape, dtype=bool)
    #translate = (0, 0, 0)
    #scale = 1
    #axis_order = axis

    vertices = scale_numpy_array(vertices, 0, _dims - 1)

    for vert in vertices:
        x = _dims - 1 - int(vert[0])
        y = int(vert[1])
        z = int(vert[2])
        data[x][y][z] = True

    #bv = binvox_rw.Voxels(data, shape, translate, scale, axis_order)

    return data

def main():
    argv = sys.argv
    argv = argv[argv.index("--") + 1:] # get all args after "--"

    inputPath1 = argv[0] # "input"
    inputExt1 = argv[1] # "_pre.ply"
    inputPath2 = argv[2] # "output"
    inputExt2 = argv[3] # "_post.ply"
    inputPath3 = argv[4] # "output"
    inputExt3 = argv[5] # "_post_edges.ply"
    dims = int(argv[6]) # 128

    colorUrls = []
    coreUrls = []
    edgeUrls = []

    for fileName in os.listdir(inputPath1):
        if fileName.endswith(inputExt1): 
            url = os.path.join(inputPath1, fileName)
            colorUrls.append(url)
    colorUrls.sort()

    for fileName in os.listdir(inputPath2):
        if fileName.endswith(inputExt2): 
            url = os.path.join(inputPath2, fileName)
            coreUrls.append(url)
    coreUrls.sort()

    for fileName in os.listdir(inputPath3):
        if fileName.endswith(inputExt3): 
            url = os.path.join(inputPath3, fileName)
            edgeUrls.append(url)
    edgeUrls.sort()

    if (len(colorUrls) == len(coreUrls) == len(edgeUrls)):
	    la = latk.Latk()
	    layer = latk.LatkLayer()
	    la.layers.append(layer)
	    for i in range(0, len(colorUrls)):
	    	frame = latk.LatkFrame(frame_number=i)
	    	la.layers[0].frames.append(frame)

		skel = skeletonize(speed_power=1.2, Euler_step_size=0.5, depth_th=2, length_th=None, simple_path=False, verbose=True)

	    for i in range(0, len(colorUrls)):
        	colorMs = ml.MeshSet()
        	colorMs.load_new_mesh(colorUrls[i])
        	colorMesh = colorMs.current_mesh()
        	colorVertices = colorMesh.vertex_matrix()

        	coreMs = ml.MeshSet()
        	coreMs.load_new_mesh(coreUrls[i])
        	coreMesh = coreMs.current_mesh()
        	coreVertices = coreMesh.vertex_matrix()

        	edgeMs = ml.MeshSet()
        	edgeMs.load_new_mesh(edgeUrls[i])
        	edgeMesh = edgeMs.current_mesh()
        	edgeVertices = edgeMesh.vertex_matrix()

			colorSk = skel.skeleton(meshToVoxels(coreVertices, dims))

			for limb in colorSk:
				points = []
				for point in limb:
					point = latk.LatkPoint((point[0], point[2], point[1]))
					points.append(point)
				stroke = latk.LatkStroke(points)
				la.layers[0].frames[i].strokes.append(stroke)

			#la.refine()

			la.write("output.latk")
	else:
		print("Couldn't find equal numbers of assets.")

main()