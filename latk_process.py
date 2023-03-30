import sys
import os
import xml.etree.ElementTree as etree
from pathlib import Path
import pymeshlab as ml
import numpy as np
import latk
from scipy.spatial.distance import cdist

'''
def distance(point1, point2):
    point1 = np.array(point1)
    point2 = np.array(point2)
    return np.linalg.norm(point1 - point2)
'''

def getBounds(mesh):
    bounds = mesh.bounding_box().diagonal() #distance(mesh.bounds[0], mesh.bounds[1])
    print("Bounds: " + str(bounds))
    return bounds

def group_points_into_strokes(points, radius, minPointsCount):
    strokes = []
    unassigned_points = set(range(len(points)))

    while len(unassigned_points) > 0:
        stroke = [next(iter(unassigned_points))]
        unassigned_points.remove(stroke[0])

        for i in range(len(points)):
            if i in unassigned_points and cdist([points[i]], [points[stroke[-1]]])[0][0] < radius:
                stroke.append(i)
                unassigned_points.remove(i)

        if (len(stroke) >= minPointsCount):
            strokes.append(stroke)
        print("Found " + str(len(strokes)) + " strokes, " + str(len(unassigned_points)) + " points remaining.")
    return strokes

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

        # The resample method can subtract points from an unstructured point cloud, 
        # but needs connection information to add them.
        if (samplePercentage > 1.0):
            if (mesh.edge_number() == 0 and mesh.face_number() == 0):
                ms.generate_surface_reconstruction_ball_pivoting()
            ms.generate_sampling_poisson_disk(samplenum=newSampleNum, subsample=False)
        else:
            ms.generate_sampling_poisson_disk(samplenum=newSampleNum, subsample=True)
        ms.transfer_attributes_per_vertex(sourcemesh=0, targetmesh=1)
        mesh = ms.current_mesh()
        
        '''
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
        '''
        bounds = getBounds(mesh)
        searchRadius = bounds * 0.02
        
        strokes = group_points_into_strokes(mesh.vertex_matrix(), searchRadius, minStrokePoints) #mesh.vertices, searchRadius)

        for stroke in strokes:
            la_stroke = latk.LatkStroke()
            for index in stroke:
                vert = mesh.vertex_matrix()[index]
                la_point = latk.LatkPoint(co=(vert[0], vert[2], vert[1]))
                la_stroke.points.append(la_point)
            la.layers[0].frames[counter].strokes.append(la_stroke)

        # ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

        allPoints = []
        for stroke in la.layers[0].frames[counter].strokes:
            for point in stroke.points:
                allPoints.append([point.co[0], point.co[2], point.co[1]])
        vertices = np.array(allPoints)
        faces = np.array([[0,0,0]])
        mesh = ml.Mesh(vertices, faces)
        ms.add_mesh(mesh)
        ms.transfer_attributes_per_vertex(sourcemesh=1, targetmesh=2)
        mesh = ms.current_mesh()

        strokeCounter = 0
        pointCounter = 0
        vertexColors = mesh.vertex_color_matrix()

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

    la.normalize()
    #la.refine() #splitReps=0, smoothReps=20, reduceReps=0, doClean=True)
    la.write("output.latk")
    print("Wrote latk")

main()
