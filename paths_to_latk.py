import sys
import os
import numpy as np
import latk
from random import uniform as rnd
import pymeshlab as ml
import distutils.util
import trimesh
import networkx as nx

def scale_numpy_array(arr, min_v, max_v):
    new_range = (min_v, max_v)
    max_range = max(new_range)
    min_range = min(new_range)

    scaled_unit = (max_range - min_range) / (np.max(arr) - np.min(arr))
    return arr * scaled_unit - np.min(arr) * scaled_unit + min_range

def scaleVertices(vertices, _dims=128):
    return scale_numpy_array(vertices, 0, _dims - 1)

def main():
    argv = sys.argv
    argv = argv[argv.index("--") + 1:] # get all args after "--"

    inputPath = argv[0] # "output"
    inputExt = argv[1] # "_final.ply"
    dims = int(argv[2]) # 128

    urls = []

    for fileName in os.listdir(inputPath):
        if fileName.endswith(inputExt): 
            url = os.path.abspath(os.path.join(inputPath, fileName))
            urls.append(url)
    urls.sort()

    la = latk.Latk()
    layer = latk.LatkLayer()
    la.layers.append(layer)
    for i in range(0, len(urls)):
        frame = latk.LatkFrame(frame_number=i)
        la.layers[0].frames.append(frame)

    for i in range(0, len(urls)):
        print("\nLoading meshes " + str(i+1) + " / " + str(len(urls)))
        ms = ml.MeshSet()
        ms.load_new_mesh(urls[i]) # first mesh -> index 0
        coreMesh = ms.current_mesh()
        #coreVertices = coreMesh.vertex_matrix()
        #coreVertices = scaleVertices(coreVertices, dims)

        ms.add_mesh(coreMesh) # duplicates the current mesh -> index 1
        ms.surface_reconstruction_ball_pivoting()
        ms.vertex_attribute_transfer(sourcemesh=0, targetmesh=1)
        newTempUrl = os.path.abspath(os.path.join(inputPath, "output" + str(i) + ".ply"));
        ms.save_current_mesh(newTempUrl)
        vertices = ms.current_mesh().vertex_matrix()
        vertexColors = ms.current_mesh().vertex_color_matrix()

        print("\nSorting edges " + str(i+1) + " / " + str(len(urls)))
        # https://trimsh.org/examples/shortest.html
        mesh = trimesh.load(newTempUrl)
        # edges without duplication
        edges = mesh.edges_unique
        # the actual length of each unique edge
        length = mesh.edges_unique_length
        # create the graph with edge attributes for length
        g = nx.Graph()
        for edge, L in zip(edges, length):
            g.add_edge(*edge, length=L)

        print("\nConnecting edge points " + str(i+1) + " / " + str(len(urls)))     
        
        numStrokes = 10
        maxStrokePoints = 10

        for j in range(0, numStrokes):
            try:
                start = int(rnd(maxStrokePoints, len(mesh.vertices) - maxStrokePoints))
                end = start + int(rnd(-maxStrokePoints, maxStrokePoints))
                # run the shortest path query using length for edge weight
                path = nx.shortest_path(g, source=start, target=end, weight='length')

                points = []
                for index in path:
                    vert = mesh.vertices[index]
                    col = mesh.visual.vertex_colors[index]

                    # TODO find out why colors are too light
                    col = (col[0] * col[0], col[1] * col[1], col[2] * col[2], col[3])

                    #print(col)
                    newVert = (-vert[0], vert[2], vert[1])
                    lp = latk.LatkPoint(newVert, vertex_color=(col[0], col[1], col[2], col[3]))
                    points.append(lp)
                stroke = latk.LatkStroke(points)
                la.layers[0].frames[i].strokes.append(stroke)
                print ("Stroke " + str(j+1) + " / " + str(numStrokes))
            except:
                print ("Skipped " + str(j+1) + " / " + str(numStrokes))

    print("\nWriting latk...")
    la.write("output.latk")
    print("\n...Finished writing latk.")

main()