import sys
import distutils.util
import mesh_converter as mc
import os

def main():
    argv = sys.argv
    argv = argv[argv.index("--") + 1:] # get all args after "--"

    inputPath = argv[0]
    ext = argv[1]
    dims = int(argv[2])

    for fileName in os.listdir(inputPath):
        if fileName.endswith(ext): 
            url = os.path.join(inputPath, fileName)

            mc.binvoxToMesh(url=url, dims=dims)

main()