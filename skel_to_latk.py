import sys
import os
import numpy as np
from skeletonize import skeletonize
import latk
import binvox_rw
from random import uniform as rnd
import pymeshlab as ml

def scale_numpy_array(arr, min_v, max_v):
    new_range = (min_v, max_v)
    max_range = max(new_range)
    min_range = min(new_range)

    scaled_unit = (max_range - min_range) / (np.max(arr) - np.min(arr))
    return arr * scaled_unit - np.min(arr) * scaled_unit + min_range

def scaleVertices(vertices, _dims=128):
    return scale_numpy_array(vertices, 0, _dims - 1)

def meshToVoxels(vertices=None, _dims=128): #, axis='xyz'):
    shape = (_dims, _dims, _dims)
    data = np.zeros(shape, dtype=bool)

    for vert in vertices:
        x = int(vert[0])
        y = int(vert[1])
        z = int(vert[2])
        data[x][y][z] = True

    return data

def main():
    argv = sys.argv
    argv = argv[argv.index("--") + 1:] # get all args after "--"

    inputPath = argv[0] # "output"
    inputExt = argv[1] # "final.obj"
    dims = int(argv[2]) # 128

    urls = []

    for fileName in os.listdir(inputPath):
        if fileName.endswith(inputExt): 
            url = os.path.join(inputPath, fileName)
            urls.append(url)
    urls.sort()

    la = latk.Latk()
    layer = latk.LatkLayer()
    la.layers.append(layer)
    for i in range(0, len(urls)):
        frame = latk.LatkFrame(frame_number=i)
        la.layers[0].frames.append(frame)

    #skel = skeletonize(speed_power=1.2, Euler_step_size=0.5, depth_th=2, length_th=None, simple_path=False, verbose=True)
    skel = skeletonize(speed_power=1.2, Euler_step_size=0.5, depth_th=3, length_th=None, simple_path=False, verbose=True)

    for i in range(0, len(urls)):
        print("\nLoading meshes " + str(i+1) + " / " + str(len(colorUrls)))
        ms = ml.MeshSet()
        ms.load_new_mesh(urls[i])
        coreMesh = ms.current_mesh()
        coreVertices = coreMesh.vertex_matrix()
        coreVertices = scaleVertices(coreVertices, dims)
        coreColors = coreMesh.color_matrix()

        ms.load_new_mesh(urls[i])
        ms.surface_reconstruction_ball_pivoting()
        ms.select_crease_edges()
        ms.build_a_polyline_from_selected_edges()

        edgeMesh = ms.current_mesh()
        edgeVertices = edgeMesh.vertex_matrix()
        edgeVertices = scaleVertices(edgeVertices, dims)
        edgeEdges = edgeMesh.edge_matrix()

        ms.vertex_attribute_transfer(sourcemesh=0, targetmesh=1)
        edgeColors = edgeMesh.color_matrix()

        print("\nCore skeleton " + str(i+1) + " / " + str(len(colorUrls)))
        coreSk = skel.skeleton(meshToVoxels(coreVertices, dims))

        for limb in coreSk:
            points = []
            for point in limb:
                point = latk.LatkPoint((point[0], point[2], point[1]))
                points.append(point)
            stroke = latk.LatkStroke(points)
            la.layers[0].frames[i].strokes.append(stroke)

        print("\nEdge detail " + str(i+1) + " / " + str(len(colorUrls)))          
        strokes = []

        for edge in edgeEdges:
            points = []
            for edgePoint in edge:
                vert = edgeVertices[edgePoint]
                col = edgeColors[edgePoints]
                points.append(latk.LatkPoint((vert[0], vert[2], vert[1]), vertex_color = col))
            stroke = latk.LatkStroke(points)
            strokes.append(stroke)

        newStrokes = []
        newStrokeDistance = (dims / 50.0)
        minStrokePoints = int(dims / 5)

        for j, stroke in enumerate(strokes):
            isNewStroke = True

            if (j > 0):
                for point in stroke.points:
                    for newPoint in newStrokes[len(newStrokes)-1].points:
                        if la.getDistance(point.co, newPoint.co) < newStrokeDistance: 
                            isNewStroke = False
                            break
                    if (isNewStroke == False):
                        break

            if (isNewStroke == True):
                newStrokes.append(stroke)
            else:
                for point in stroke.points:
                    newStrokes[len(newStrokes)-1].points.append(point)

        for stroke in newStrokes:
            if (len(stroke.points) > minStrokePoints):
                startPoint = points[0]
                for point in stroke.points:
                    point.distance = la.getDistance(point.co, startPoint.co)

            stroke.points = sorted(stroke.points, key=lambda x: getattr(x, 'distance'))
            la.layers[0].frames[i].strokes.append(stroke)

    print("\nWriting latk...")
    la.write("output.latk")
    print("\n...Finished writing latk.")

main()