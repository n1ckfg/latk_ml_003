inputPath = argv[0] # "output"
inputExt = argv[1] # "_final.ply"

1. Works.
    dims = int(argv[2]) # 256
    doSkeleton = bool(distutils.util.strtobool(argv[3]))

2. WORKS, IS INTERESTING.
    dims = int(argv[2]) # 256
    numStrokes = int(argv[3]) #100
    minStrokePoints = int(argv[4]) #10
    maxStrokePoints = int(argv[5]) #9999
    maxSimilarity = float(argv[6]) #0.8

3. WORKS, IS INTERESTING; similar to 2.
    dims = int(argv[2]) # 256
    numStrokes = int(argv[3]) #100
    minStrokePoints = int(argv[4]) #10
    maxStrokePoints = int(argv[5]) #9999
    maxSimilarity = float(argv[6]) #0.8

4. Works.
    numClusters = int(argv[2]) # 10
    dims = int(argv[3]) # 256
    numStrokes = int(argv[4]) #100
    minStrokePoints = int(argv[5]) #10
    maxStrokePoints = int(argv[6]) #9999
    maxSimilarity = float(argv[7]) #0.8

5. Works.
    numClusters = int(argv[2]) # 10
    dims = int(argv[3]) # 256
    numStrokes = int(argv[4]) #100
    minStrokePoints = int(argv[5]) #10
    maxStrokePoints = int(argv[6]) #9999
    maxSimilarity = float(argv[7]) #0.8

6. Works.
    numClusters = int(argv[2]) # 10
    dims = int(argv[3]) # 256    
    numStrokes = int(argv[4]) #100
    minStrokePoints = int(argv[5]) #10
    maxStrokePoints = int(argv[6]) #9999

7. Requires SynDraw.
    samplePercentage = float(argv[2]) #0.1
    minStrokePoints = int(argv[3]) #2
