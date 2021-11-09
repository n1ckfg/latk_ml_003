import sys
import os
import latk
import trimesh

def main():
    argv = sys.argv
    argv = argv[argv.index("--") + 1:] # get all args after "--"

    inputPath1 = argv[0]

    for fileName in os.listdir(inputPath1):
        if fileName.endswith(argv[1]): 
            url = os.path.join(inputPath1, fileName)

            mesh = trimesh.load(url)

            points = []

main()