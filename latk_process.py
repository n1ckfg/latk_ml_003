import sys
import os
import xml.etree.ElementTree as etree
from pathlib import Path
import pymeshlab as ml
import numpy as np
import latk

def main():
    argv = sys.argv
    argv = argv[argv.index("--") + 1:] # get all args after "--"

    inputPath = argv[0] # "output"
    inputExt = argv[1] # "_final.ply"
    samplePercentage = float(argv[2]) #0.1
    minStrokePoints = int(argv[3]) #2

    la = latk.Latk()
    la.layers.append(latk.LatkLayer())
    counter = 0

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
        ms.load_new_mesh(urls[i])
        mesh = ms.current_mesh()

        newSampleNum = int(mesh.vertex_number() * samplePercentage)
        ms.generate_simplified_point_cloud(samplenum=newSampleNum)
        ms.generate_surface_reconstruction_ball_pivoting()
        ms.transfer_attributes_per_vertex(sourcemesh=0, targetmesh=1)
        ms.save_current_mesh("input.ply", save_vertex_color=True)

        os.system("SynDraw -p test.template")

        print("parsing SVG")
        tree = etree.parse("out.svg")
        root = tree.getroot()
        for element in root:
            if (element.tag.endswith("polyline")):
                points = element.get("points3d").split(" ")
                lPoints = []
                for point in points:
                    point2 = point.split(",")
                    try:
                        point3 = (float(point2[0]), float(point2[2]), float(point2[1]))
                        lPoints.append(latk.LatkPoint(point3))
                    except:
                        pass
                if (len(lPoints) >= minStrokePoints):
                    la.layers[0].frames[counter].strokes.append(latk.LatkStroke(lPoints))

        allPoints = []
        for stroke in la.layers[0].frames[counter].strokes:
            for point in stroke.points:
                allPoints.append([point.co[0], point.co[2], point.co[1]])
        vertices = np.array(allPoints)
        faces = np.array([[0,0,0]])
        mesh = ml.Mesh(vertices, faces)
        ms.add_mesh(mesh)
        ms.transfer_attributes_per_vertex(sourcemesh=1, targetmesh=2)
        
        strokeCounter = 0
        pointCounter = 0
        vertexColors = ms.current_mesh().vertex_color_matrix()

        for vertexColor in vertexColors:
            color = (vertexColor[0], vertexColor[1], vertexColor[2], 1.0)
            color = (color[0] * color[0], color[1] * color[1], color[2] * color[2], 1.0)
            la.layers[0].frames[counter].strokes[strokeCounter].points[pointCounter].vertex_color = color
            pointCounter += 1
            if (pointCounter > len(la.layers[0].frames[counter].strokes[strokeCounter].points)-1):
                pointCounter = 0
                strokeCounter += 1 

        print("Finished frame " + str(counter+1))
        counter += 1

    la.write("output.latk")
    print("Wrote latk")

main()
