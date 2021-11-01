import sys
import os
import pymeshlab as ml

def main():
    argv = sys.argv
    argv = argv[argv.index("--") + 1:] # get all args after "--"

    inputPath1 = argv[0]

    for fileName in os.listdir(inputPath1):
        if fileName.endswith(argv[1]): 
            url = os.path.join(inputPath1, fileName)

            newPathBase = fileName.split(argv[1])[0]
            inputPath2 = os.path.join(argv[2], newPathBase + argv[3])
            outputPath = os.path.join(argv[2], newPathBase + "_final.obj")

            ms = ml.MeshSet()
            ms.load_new_mesh(os.path.abspath(url)) # pymeshlab needs absolute paths
            ms.load_new_mesh(os.path.abspath(inputPath2)) # pymeshlab needs absolute paths
            
            # https://pymeshlab.readthedocs.io/en/latest/filter_list.html    
            ms.apply_filter("vertex_attribute_transfer", sourcemesh=0, targetmesh=1)
            ms.save_current_mesh(os.path.abspath(outputPath)) # pymeshlab needs absolute paths

main()