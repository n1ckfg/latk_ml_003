import sys
import os
import latk
import pymeshlab as ml

def main():
    argv = sys.argv
    argv = argv[argv.index("--") + 1:] # get all args after "--"

    inputPath1 = argv[0]

    la = latk.Latk()
    layer = latk.LatkLayer()
    la.layers.append(layer)

    frameCounter = 0
    maxStrokePoints = 100

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

            strokes = []
            for i in range(0, len(points) - maxStrokePoints, maxStrokePoints):
            	newPoints = []
            	for j in range(0, maxStrokePoints):
            		newPoints.append(points[i+j])
            	stroke = latk.LatkStroke(newPoints)
            	strokes.append(stroke)
            
            frame = latk.LatkFrame(strokes)

            la.layers[0].frames.append(frame)

    la.refine()

    print("Writing latk...")
    la.write("output.latk")

main()