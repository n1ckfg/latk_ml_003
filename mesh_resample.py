import sys
import os
import pymeshlab as ml

def changeExtension(_url, _newExt):
    returns = ""
    returnsPathArray = _url.split(".")
    for i in range(0, len(returnsPathArray)-1):
        returns += returnsPathArray[i]
    returns += _newExt
    return returns

def main():
    argv = sys.argv
    argv = argv[argv.index("--") + 1:] # get all args after "--"

    inputPath = argv[0]
    samplePercentage = float(argv[1])
    inputFormat = argv[2]
    outputFormat = argv[3]

    for fileName in os.listdir(inputPath):
        if fileName.endswith(inputFormat): 
            inputUrl = os.path.join(inputPath, fileName)
            print("Resampling " + inputUrl)
            
            outputUrl = changeExtension(inputUrl, outputFormat)

            ms = ml.MeshSet()
            ms.load_new_mesh(os.path.abspath(inputUrl)) # pymeshlab needs absolute paths
            mesh = ms.current_mesh()

            newSampleNum = int(mesh.vertex_number() * samplePercentage)
            if (newSampleNum < 1):
                newSampleNum = 1

            # The resample method can subtract points from an unstructured point cloud, 
            # but needs connection information to add them.
            if (samplePercentage > 1.0):
                if (mesh.edge_number() == 0 and mesh.face_number() == 0):
                    ms.surface_reconstruction_ball_pivoting()
                ms.poisson_disk_sampling(samplenum=newSampleNum, subsample=False)
            else:
                ms.poisson_disk_sampling(samplenum=newSampleNum, subsample=True)
                
            ms.save_current_mesh(os.path.abspath(outputUrl)) # pymeshlab needs absolute paths

main()