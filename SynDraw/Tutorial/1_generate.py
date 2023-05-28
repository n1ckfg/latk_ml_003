import sys
import numpy as np 
from subprocess import Popen, PIPE
from pyjavaproperties import Properties
from datetime import datetime
import argparse

LOG = True

SYNDRAW_EXE="../SynDraw/build/SynDraw"
NORMALS_EXE="../SynDraw/build/SynDraw_Normal"
RESAMPLER_PY="../svg_tools/svg_resampler.py" 
DISTURBER_PY="../svg_tools/svg_disturber.py"
RASTERIZER_PY="../svg_tools/svg_rasterizer.py"

OFF_DIR="./off/"
SVG_DIR="./svg/"
PNG_DIR='./png/'
TMP_DIR="./tmp/"
NML_DIR="./normals/"
PPT_DIR="./properties/"

# Viewpoints are the 4 top corners of a 4x4x2 cuboid \
# that were slightly displaced to the left and to the right
COORDINATES={
	'x': [1.5, 1.7, -1.5, -1.7],
	'y': [1.5, 1.7, -1.5, -1.7],
	'z': [2]
}
VIEWPOINTS = []

SVG_P = {
	'dist_mean': 2,
	'dist_variance': 0.4,
	'pen_width': 1.5,
	'pen_variance': 0.5,
	'per_point_noise': 0.7,
	'max_strokes': 2,
	'oversketching': 4,
	# more parameters exist ! check out 'python svg_disturber.py -help'
}

# Build all viewpoints position list
def build_viewpoints():
	for x in COORDINATES['x']:
		for y in COORDINATES['y']:
			for z in COORDINATES['z']:
				if abs(x) != abs(y):
					VIEWPOINTS.append([x, y, z])

# Run a bash command in current session
# cmd is a list of all arguments, e.g. ['ls', '-al', './folder']
def bash_cmd(cmd, verbose=False):
	if LOG:
		print '[', datetime.now(), ']  ',
		for s in cmd: print s,
		print

	if verbose:
		p = Popen(cmd, shell=False, stderr=sys.stderr, stdout=sys.stdout)
	else:
		p = Popen(cmd, shell=False, stderr=PIPE, stdout=PIPE)
	stdout, stderr = p.communicate() 	
	return [stdout, stderr]

# Generate the SynDraw properties file for the designated entry
# based on the template in this folder
def prepare_properties(index, nb):
	vp = VIEWPOINTS[nb]
	p = Properties()
	p.load(open("./template.properties"))

	# Input mesh path
	p["mesh_in"] = "%s%s.off" % (OFF_DIR, index)

	# Output SVG path
	p["svg_out"] = "%s%s_%s.svg" % (SVG_DIR, index, nb)

	# Camera position used
	p["camera_position"] = "%s, %s, %s" % (vp[0], vp[1], vp[2])

	# Path to this entry's properties file
	path = "%s%s_%s.properties" % (PPT_DIR, index, nb)
	p.store(open(path, 'w'))
	return path

# Generate SVG and normal map from mesh using SynDraw
def syndraw_one(index, nb):
	ppath = prepare_properties(index, nb)
	bash_cmd([SYNDRAW_EXE, "-p", ppath])

	npath = "%s%s_%s.png" % (NML_DIR, index, nb)
	bash_cmd([NORMALS_EXE, "-p", ppath, "-o", npath, "-m", "2", "-b", "1"])


# Apply stylization step to a single entry
# The SVG is resampled, then noise is applied
def stylize_one(index, nb):
	# Input SVG path
	svg = "%s%s_%s.svg" % (SVG_DIR, index, nb)

	# Output PNG path
	png = "%s%s_%s.png" % (PNG_DIR, index, nb)

	# Temporary file
	tmp1 = "%s%s_%s_1resampled.svg" % (TMP_DIR, index, nb)
	tmp2 = "%s%s_%s_2noisy.svg" % (TMP_DIR, index, nb)

	# Resampling
	d = str(SVG_P['dist_mean'])
	m = str(SVG_P['dist_mean'] - SVG_P['dist_variance']/2.0)
	M = str(SVG_P['dist_mean'] + SVG_P['dist_variance']/2.0)
	bash_cmd(["python", RESAMPLER_PY, svg, tmp1, d, m, M]) 

	# Adding noise
	pen = str(SVG_P['pen_width'])
	penv = str(SVG_P['pen_variance'])
	n = str(SVG_P['per_point_noise'])
	maxs = str(SVG_P['max_strokes'])
	os = str(SVG_P['oversketching'])
	bash_cmd(["python", DISTURBER_PY, tmp1, tmp2, '-pen', pen, '-penv', penv, '-n', n, '-max', maxs, '-os', os, '-c', '-u', '-f'])

	# Rasterizing SVG
	bash_cmd(['python', RASTERIZER_PY, tmp2, png])



# Run the pipeline for a single entry
# From the mesh, we generate a synthetic sketch and an aligned normal map
def doall_one(index, nb):
	syndraw_one(index, nb)
	stylize_one(index, nb)


if __name__ == "__main__":
	parser = argparse.ArgumentParser()   
	parser.add_argument("-start", help="start value", type=int, default=0)
	parser.add_argument("-nb_data", help="total number of images to use", type=int, default=20000)
	args = parser.parse_args()

	build_viewpoints()

	for i in range(args.start, args.nb_data):
		for v in range(0, len(VIEWPOINTS)):
			doall_one(i,v)

