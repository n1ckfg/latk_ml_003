import sys
import os
import pymeshlab as ml

def main():
    argv = sys.argv
    argv = argv[argv.index("--") + 1:] # get all args after "--"

    inputPath1 = argv[0]
    newPathBase = os.path.basename(inputPath1)
    newPathBase = newPathBase.split("_pre")[0]
    inputPath2 = os.path.join(argv[1], newPathBase + "_fake_filter_post_edges.ply")
    outputPath = os.path.join(argv[1], newPathBase + "_final.ply")

    ms = ml.MeshSet()
    ms.load_new_mesh(inputPath1)
    ms.load_new_mesh(inputPath2)
    
    # https://pymeshlab.readthedocs.io/en/latest/filter_list.html    
    ms.apply_filter("vertex_attribute_transfer", sourcemesh=0, targetmesh=1)
    ms.save_current_mesh(outputPath)

main()