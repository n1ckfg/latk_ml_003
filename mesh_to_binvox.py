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
    urls = []

    for fileName in os.listdir(inputPath):
        if fileName.endswith(inputExt): 
            url = os.path.join(inputPath, fileName)
            print("Finding bounds for " + url)

            urls.append(url)
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

            localDim = (minX, maxX, minY, maxY, minZ, maxZ)
            #print(localDim)
            localDims.append(localDim)

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
        normMinX = mc.remap(localDim[0], seqMinX, seqMaxX, 0, 1)
        normMaxX = mc.remap(localDim[1], seqMinX, seqMaxX, 0, 1)
        normMinY = mc.remap(localDim[2], seqMinY, seqMaxY, 0, 1)
        normMaxY = mc.remap(localDim[3], seqMinY, seqMaxY, 0, 1)
        normMinZ = mc.remap(localDim[4], seqMinZ, seqMaxZ, 0, 1)
        normMaxZ = mc.remap(localDim[5], seqMinZ, seqMaxZ, 0, 1)

        normVals = (normMinX, normMaxX, normMinY, normMaxY, normMinZ, normMaxZ)
        #print(normVals)

        localNorms.append(normVals)

    for i, url in enumerate(urls):
        #mc.meshToBinvox(url=url, ext=outputExt, dims=dims, doFilter=doFilter)
        mc.meshToBinvox(url=url, ext=outputExt, dims=dims, doFilter=doFilter, normVals=localNorms[i], dimVals=localDims[i])

main()