import sys
import os
import distutils.util
import mesh_converter as mc
import trimesh

def main():
    argv = sys.argv
    argv = argv[argv.index("--") + 1:] # get all args after "--"

    inputPath = argv[0]
    inputExt = argv[1]
    outputExt = argv[2]
    dims = int(argv[3])
    doFilter = bool(distutils.util.strtobool(argv[4]))

    seqMinX = 0
    seqMaxX = 0
    seqMinY = 0
    seqMaxY = 0
    seqMinZ = 0
    seqMaxZ = 0

    seqMin = 0
    seqMax = 0

    localDims = []
    localPercentages = []

    for fileName in os.listdir(inputPath):
        if fileName.endswith(inputExt): 
            url = os.path.join(inputPath, fileName)
            mesh = trimesh.load(url)
            minX = 0
            maxX = 0
            minY = 0
            maxY = 0
            minZ = 0
            maxZ = 0
            for vert in mesh.vertices:
                x = vert[0]
                y = vert[1]
                z = vert[2]
                if (x < minX):
                    minX = x
                if (x > maxX):
                    maxX = x
                if (y < minY):
                    minY = y
                if (y > maxY):
                    maxY = y
                if (z < minZ):
                    minZ = z
                if (z > maxZ):
                    maxZ = z

            localDims.append((minX, maxX, minY, maxY, minZ, maxZ))

            if (minX < seqMinX):
                seqMinX = minX
            if (maxX > seqMaxX):
                seqMaxX = maxX
            if (minY < seqMinY):
                seqMinY = minY
            if (maxY > seqMaxY):
                seqMaxY = maxY
            if (minZ < seqMinZ):
                seqMinZ = minZ
            if (maxZ > seqMaxZ):
                seqMaxZ = maxZ

    seqMinArray = [seqMinX, seqMinY, seqMinZ]
    seqMinArray.sort()
    seqMin = seqMinArray[0]
    seqMaxArray = [seqMaxX, seqMaxY, seqMaxZ]
    seqMaxArray.sort()
    seqMax = seqMaxArray[2]

    for dims in localDims:
        minValArray = [dims[0], dims[2], dims[4]]
        minValArray.sort()
        minVal = minValArray[0]
        maxValArray = [dims[1], dims[3], dims[5]]
        maxValArray.sort()
        maxVal = maxValArray[2]

        percentage = (minVal / seqMin, maxVal / seqMax)
        print(percentage)
        
        localPercentages.append(percentage)

    for fileName in os.listdir(inputPath):
        if fileName.endswith(inputExt): 
            url = os.path.join(inputPath, fileName)
            mc.meshToBinvox(url=url, ext=outputExt, dims=dims, doFilter=doFilter)

main()