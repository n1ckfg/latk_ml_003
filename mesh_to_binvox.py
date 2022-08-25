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

    seqMin = 0.0
    seqMax = 0.0

    urls = []

    for fileName in os.listdir(inputPath):
        if fileName.endswith(inputExt): 
            url = os.path.join(inputPath, fileName)
            print("Finding bounds for " + url)

            urls.append(url)
            mesh = trimesh.load(url)
                       
            for vert in mesh.vertices:
                x = vert[0]
                y = vert[1]
                z = vert[2]
                if (x < seqMin):
                    seqMin = x
                if (x > seqMax):
                    seqMax = x
                if (y < seqMin):
                    seqMin = y
                if (y > seqMax):
                    seqMax = y
                if (z < seqMin):
                    seqMin = z
                if (z > seqMax):
                    seqMax = z

    for i, url in enumerate(urls):
        mc.meshToBinvox(url=url, ext=outputExt, dims=dims, doFilter=doFilter, seqMin=seqMin, seqMax=seqMax)

main()