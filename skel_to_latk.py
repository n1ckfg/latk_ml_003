import sys
import os
import numpy as np
from skeletonize import skeletonize
import latk
import binvox_rw
from random import uniform as rnd
import pymeshlab as ml
import distutils.util

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
    inputExt = argv[1] # "_final.ply"
    dims = int(argv[2]) # 128
    doSkeleton = bool(distutils.util.strtobool(argv[3]))

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

    skel = None
    if (doSkeleton == True):
        skel = skeletonize(speed_power=1.2, Euler_step_size=0.5, depth_th=2, length_th=None, simple_path=False, verbose=True)

    for i in range(0, len(urls)):
        print("\nLoading meshes " + str(i+1) + " / " + str(len(urls)))
        ms = ml.MeshSet()
        ms.load_new_mesh(urls[i]) # first mesh -> index 0
        coreMesh = ms.current_mesh()
        coreVertices = coreMesh.vertex_matrix()
        coreVertices = scaleVertices(coreVertices, dims)

        ms.add_mesh(coreMesh) # duplicates the current mesh -> index 1
        samplePercentage = 0.1
        newSampleNum = int(ms.current_mesh().vertex_number() * samplePercentage)
        if (newSampleNum < 1):
            newSampleNum = 1

        ms.apply_filter("poisson_disk_sampling", samplenum=newSampleNum, subsample=True)
        ms.surface_reconstruction_ball_pivoting()
        ms.select_crease_edges()
        ms.build_a_polyline_from_selected_edges() # this command creates a new mesh -> index 2

        ms.apply_filter("vertex_attribute_transfer", sourcemesh=0, targetmesh=2)
        edgeMesh = ms.current_mesh()
        edgeVertices = edgeMesh.vertex_matrix()
        edgeVertices = scaleVertices(edgeVertices, dims)
        edgeEdges = edgeMesh.edge_matrix()
        edgeColors = edgeMesh.vertex_color_matrix()

        # ~ ~ ~ ~ ~ ~ ~ ~ ~
        print("\nSorting edges " + str(i+1) + " / " + str(len(urls)))
        # https://blenderscripting.blogspot.com/2011/07/sorting-edge-keys-part-ii.html

        # existing edges    
        ex_edges = []
        existing_edges = []
        for i in edgeEdges:
            edge_keys = [i[0], i[1]]
            ex_edges.append(edge_keys)
            item = [i.index, edge_keys] 
            existing_edges.append(item)
            print(item)
    
        # proposed edges
        proposed_edges = []    
        num_edges = len(existing_edges)
        for i in range(num_edges):
            item2 = [i,[i,i+1]]
            proposed_edges.append(item2)
            print(item2)

        # find first end point, discontinue after finding a lose end.
        current_sequence = []
        iteration = 0
        while (iteration <= num_edges):
            count_presence = 0
            for i in edgeEdges:
                try:
                    if iteration in i[1]:
                        count_presence += 1
                except:
                    pass
            
            print("iteration: ", iteration, count_presence)
            if count_presence == 1:
                break
            iteration += 1
            
        init_num = iteration
        print("end point", init_num)

        # find connected sequence
        seq_list = []
        glist = []

        def generate_ladder(starter, edge_key_list):
            
            def find_vert_connected(vert, mlist):
                if len(mlist) == 1:
                   for g in mlist:
                        for k in g:
                            if k is not vert:
                                return(k, -1)
            
                for i in mlist:
                    if vert in i:
                        idx = mlist.index(i)
                        for m in i:
                            if m is not vert:
                                return(m, idx)

            stairs = []
            while(True):
                stairs.append(starter)
                starter, idx = find_vert_connected(starter,  edge_key_list)
                if idx == -1:
                    stairs.append(starter)
                    break
                edge_key_list.pop(idx)
            return(stairs)

        seq_list = generate_ladder(init_num, ex_edges)

        # make verts and edges
        newVerts = []
        newEdges = []

        for i in range(len(edgeVertices)):
            print(i)
            old_idx = seq_list[i]
            myVec = edgeVertices[old_idx]
            newVerts.append((myVec[0], myVec[i], myVec[2]))
            
        for i in newVerts: print(i)

        for i in proposed_edges:
            newEdges.append(tuple(i[1]))
        print(newEdges)  
        # ~ ~ ~ ~ ~ ~ ~ ~ ~

        if (doSkeleton == True):
            print("\nGenerating skeleton " + str(i+1) + " / " + str(len(urls)))
            coreSk = skel.skeleton(meshToVoxels(coreVertices, dims))

            for limb in coreSk:
                points = []
                for point in limb:
                    point = latk.LatkPoint((-point[0], point[2], point[1]))
                    points.append(point)
                stroke = latk.LatkStroke(points)
                la.layers[0].frames[i].strokes.append(stroke)

        print("\nConnecting edge points " + str(i+1) + " / " + str(len(urls)))          
        strokes = []

        for edge in edgeEdges:
            points = []
            for edgePoint in edge:
                vert = edgeVertices[edgePoint]
                col = edgeColors[edgePoint]
                # TODO find out why colors are too light
                col = (col[0] * col[0], col[1] * col[1], col[2] * col[2], col[3])
                #print(col)
                points.append(latk.LatkPoint((-vert[0], vert[2], vert[1]), vertex_color=(col[0], col[1], col[2], col[3])))
            stroke = latk.LatkStroke(points)
            strokes.append(stroke)

        newStrokes = []
        newStrokeDistance = (dims / 50.0)
        minStrokePoints = int(dims / 35.0)

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