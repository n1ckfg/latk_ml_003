import sys
import os
import latk
import pymeshlab as ml
from random import uniform as rnd

def main():
    argv = sys.argv
    argv = argv[argv.index("--") + 1:] # get all args after "--"

    inputPath1 = argv[0]

    la = latk.Latk()
    layer = latk.LatkLayer()
    la.layers.append(layer)

    frameCounter = 0
    maxStrokePoints = 100
    maxStrokeDistance = 0.5

    for fileName in os.listdir(inputPath1):
        if fileName.endswith(argv[1]): 
            url = os.path.join(inputPath1, fileName)
            print("Loading from " + url)

            ms = ml.MeshSet()
            ms.load_new_mesh(url)
            mesh = ms.current_mesh()
            vertices = mesh.vertex_matrix()
            print("Found " + str(len(vertices)) + " vertices")

            points = []
            for vert in vertices:
                point = latk.LatkPoint((vert[0], vert[2], vert[1]))
                points.append(point)

            numStrokes = int(len(points) / maxStrokePoints)
            strokes = []
            for i in range(0, 100):
                randomStartIndex = int(rnd(0, len(points)))
                randomStartPoint = points[randomStartIndex]
                
                for point in points:
                    point.distance = la.getDistance(point.co, randomStartPoint.co)

                points = sorted(points, key=lambda x: getattr(x, 'distance'))

                newPoints = []
                for j in range(1, maxStrokePoints):
                    if (la.getDistance(points[i+j].co, points[i+j-1].co) < maxStrokeDistance):
                        newPoints.append(points[i+j])
                    else:
                        break
                if len(newPoints) > 1:
                    stroke = latk.LatkStroke(newPoints)
                    strokes.append(stroke)
            
            frame = latk.LatkFrame(strokes)

            la.layers[0].frames.append(frame)

    #la.refine()

    print("Writing latk...")
    la.write("output.latk")

main()