import sys
import os
from sklearn.cluster import KMeans
import numpy as np
import latk
import pymeshlab as ml
import trimesh
import networkx as nx
from difflib import SequenceMatcher
import jellyfish
from random import uniform as rnd

class Cluster(object):

    def __init__(self):
        self.points = []
        self.indices = []

def scale_numpy_array(arr, min_v, max_v):
    new_range = (min_v, max_v)
    max_range = max(new_range)
    min_range = min(new_range)

    scaled_unit = (max_range - min_range) / (np.max(arr) - np.min(arr))
    return arr * scaled_unit - np.min(arr) * scaled_unit + min_range

def scaleVertices(vertices, _dims=128):
    return scale_numpy_array(vertices, 0, _dims - 1)

# https://stackoverflow.com/questions/17388213/find-the-similarity-metric-between-two-strings
def similar(a, b):
    return SequenceMatcher(None, a, b).ratio()

def similar2(a, b):
    return jellyfish.jaro_distance(a, b)


def main():
    argv = sys.argv
    argv = argv[argv.index("--") + 1:] # get all args after "--"

    inputPath = argv[0] # "output"
    inputExt = argv[1] # "_final.ply"
    numClusters = int(argv[2]) # 10
    dims = int(argv[3]) # 128
    numStrokes = int(argv[4]) #100
    minStrokePoints = int(argv[5]) #10
    maxStrokePoints = int(argv[6]) #9999
    maxSimilarity = -1    
    try:
        maxSimilarity = float(argv[7]) #0.8
    except:
        pass

    checkSimilarity = maxSimilarity > 0

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

        ms.add_mesh(coreMesh) # duplicates the current mesh -> index 1
        ms.surface_reconstruction_ball_pivoting()

        ms.select_crease_edges()
        ms.build_a_polyline_from_selected_edges() # this command creates a new mesh -> index 2
        ms.surface_reconstruction_ball_pivoting()
        ms.vertex_attribute_transfer(sourcemesh=0, targetmesh=2)

        newTempUrl = os.path.abspath(os.path.join(inputPath, "output" + str(i) + ".ply"))
        ms.save_current_mesh(newTempUrl)

        mesh = trimesh.load(newTempUrl)
        edges = mesh.edges_unique
        length = mesh.edges_unique_length
        
        g = nx.Graph()
        for edge, L in zip(edges, length):
            g.add_edge(*edge, length=L)

        vertices = scaleVertices(mesh.vertices, dims)

        #kmeans = KMeans(n_clusters=numClusters, init='k-means++', max_iter=300, n_init=10)
        #clusterData = kmeans.fit_predict(mesh.vertices)

        kmeans = KMeans(n_clusters=numClusters)
        clusterData = kmeans.fit(mesh.vertices)
       
        clusters = []
        for j in range(0, numClusters):
            clusters.append(Cluster())

        for j, label in enumerate(kmeans.labels_):
            clusters[label].points.append(mesh.vertices[j])
            clusters[label].indices.append(j)

        for cluster in clusters:
            strokeCounter = 0
            similarityScores = []

            while strokeCounter < int(numStrokes / len(clusters)):
                points = []
                similarity = []

                start = int(rnd(0, len(cluster.indices) - 1))
                end = int(rnd(0, len(cluster.indices) - 1))

                try:
                    # run the shortest path query using length for edge weight
                    path = nx.shortest_path(g, source=start, target=end, weight='length')

                    for index in path:
                        vert = mesh.vertices[index] #vertices[index]
                        similarity.append(str(index))

                        col = mesh.visual.vertex_colors[index]
                        col = (float(col[0])/255.0, float(col[1])/255.0, float(col[2])/255.0, float(col[3])/255.0)
                        # TODO find out why colors are too light
                        col = (col[0] * col[0], col[1] * col[1], col[2] * col[2], col[3])

                        newVert = (-vert[0], vert[2], vert[1])
                        lp = latk.LatkPoint(newVert, vertex_color=col)
                        points.append(lp)
                        
                        if (len(points) >= maxStrokePoints):
                            break
                except:
                    print("  Frame " + str(i+1) + " / " + str(len(la.layers[0].frames)) + ", skipped stroke: No path from start to end.")
                    pass

                if (len(points) >= minStrokePoints):  
                    readyToAdd = True

                    similarityString = ' '.join(map(str, similarity))

                    if (checkSimilarity == True):
                        for score in similarityScores:
                            similarityTest = similar2(score, similarityString)
                            if (similarityTest > maxSimilarity):
                                readyToAdd = False
                                print("  Frame " + str(i+1) + " / " + str(len(la.layers[0].frames)) + ", skipped stroke: Failed similarity test (" + str(similarityTest) + ").")
                                break

                    if (readyToAdd):
                        similarityScores.append(similarityString)
                        stroke = latk.LatkStroke(points)
                        la.layers[0].frames[i].strokes.append(stroke)
                        strokeCounter += 1
                        
                        print("Frame " + str(i+1) + " / " + str(len(la.layers[0].frames)) + ", created stroke " + str(strokeCounter) + " / " + str(numStrokes) + ", with " + str(len(points)) + " points")
                else:
                    print("  Frame " + str(i+1) + " / " + str(len(la.layers[0].frames)) + ", skipped stroke: Not enough points.")
                    pass

            print("--- Frame " + str(i+1) + " / " + str(len(la.layers[0].frames)) + " FINISHED: " + str(len(la.layers[0].frames[i].strokes)) + " strokes ---")

    print("\nWriting latk...")
    la.write("output.latk")
    print("\n...Finished writing latk.")

main()