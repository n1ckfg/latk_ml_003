import sys
import distutils.util
import mesh_converter as mc
import os

def main():
    argv = sys.argv
    argv = argv[argv.index("--") + 1:] # get all args after "--"

    inputPath = argv[0]
    dims = int(argv[1])

    for fileName in os.listdir(inputPath):
        if fileName.endswith("filter.binvox"): 
            url = os.path.join(inputPath, fileName)

            mc.binvoxToMesh(url=url, dims=dims)

main()