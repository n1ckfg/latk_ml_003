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

    for fileName in os.listdir(inputPath1):
        if fileName.endswith(argv[1]): 
            url = os.path.join(inputPath1, fileName)

            ms = ml.MeshSet()
            ms.load_new_mesh(url)
            mesh = ms.current_mesh()
            vertices = mesh.vertex_matrix()

            points = []
            for vert in vertices:
                point = latk.LatkPoint((vert[0], vert[1], vert[2]))
                points.append(point)
            stroke = latk.LatkStroke(points)
            frame = latk.LatkFrame(stroke)

            la.layers[0].frames.append(frame)

    la.write("output.latk")

main()