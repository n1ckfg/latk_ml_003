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

    localDims = []
    localNorms = []

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
            allX = []
            allY = []
            allZ = []
            
            for vert in mesh.vertices:
                allX.append(vert[0])
                allY.append(vert[1])
                allZ.append(vert[2])
            
            allX.sort()
            allX.sort()
            allX.sort()
            
            minX = allX[0]
            maxX = allX[len(allX)-1]
            minY = allY[0]
            maxY = allY[len(allY)-1]
            minZ = allZ[0]
            maxZ = allZ[len(allZ)-1]

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

    for localDim in localDims:
        normMinX = 1.0 - (localDim.minX / seqMinX)
        normMaxX = localDim.maxX / seqMaxX
        normMinY = 1.0 - (localDim.minY / seqMinY)
        normMaxY = localDim.maxY / seqMaxY
        normMinZ = 1.0 - (localDim.minZ / seqMinZ)
        normMaxZ = localDim.maxZ / seqMaxZ

        normVals = (normMinX, normMaxX, normMinY, normMaxY, normMinZ, normMaxZ)
        print("Normalizing scale " + normVals)

        localNorms.append(normVals)

    counter = 0

    for fileName in os.listdir(inputPath):
        if fileName.endswith(inputExt): 
            url = os.path.join(inputPath, fileName)
            mc.meshToBinvox(url=url, ext=outputExt, dims=dims, doFilter=doFilter, normVals=localNorms[counter])
            counter += 1

main()