import sys
import os
from sklearn.cluster import KMeans
import numpy as np
import latk
import trimesh

def main():
    argv = sys.argv
    argv = argv[argv.index("--") + 1:] # get all args after "--"

    inputPath = argv[0] # "output"
    mesh = trimesh.load(inputPath)
    numClusters = 30

    kmeans = KMeans(n_clusters=numClusters, init='k-means++', max_iter=300, n_init=10)
    clusters = kmeans.fit_predict(mesh.vertices)

    la = latk.Latk(init=True)
    for i in range(0, len(kmeans.labels_)):
    	la.layers[0].frames[0].strokes.append(latk.LatkStroke())

    for i, label in enumerate(kmeans.labels_):
        lp = latk.LatkPoint((mesh.vertices[i]))
        la.layers[0].frames[0].strokes[label].points.append(lp)

    la.write("newtest.latk")

main()