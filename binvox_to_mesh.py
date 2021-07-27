import sys
import distutils.util
import mesh_converter as mc

def main():
    argv = sys.argv
    argv = argv[argv.index("--") + 1:] # get all args after "--"

    inputPath = argv[0]
    dims = int(argv[1])

    mc.binvoxToMesh(url=inputPath, dims=dims)

main()