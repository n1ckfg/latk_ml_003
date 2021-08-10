import sys
import distutils.util
import mesh_converter as mc

def main():
    argv = sys.argv
    argv = argv[argv.index("--") + 1:] # get all args after "--"

    inputPath = argv[0]
    inputExt = argv[1]
    dims = int(argv[2])
    doFilter = bool(distutils.util.strtobool(argv[3]))

    mc.meshToBinvox(url=inputPath, ext=inputExt, dims=dims, doFilter=doFilter)

main()