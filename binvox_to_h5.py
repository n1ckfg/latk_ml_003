# https://github.com/raahii/3dgan-chainer/blob/master/binvox_to_h5.py
# https://github.com/raahii/3dgan-chainer/blob/master/LICENSE
# Convert dataset(.binvox) to numpy array in advance
# to load dataset more efficient.
#
import sys, os, glob
#from common.data_io import read_binvox, read_h5
import numpy as np
import scipy.ndimage as nd
import h5py
import binvox_rw

def read_h5(path):
    """
    read .h5 file
    """
    f = h5py.File(path, 'r')
    voxel = f['data'][:]
    f.close()

    return voxel

def resize(voxel, shape):
    """
    resize voxel shape
    """
    ratio = shape[0] / voxel.shape[0]
    voxel = nd.zoom(voxel,
            ratio,
            order=1, 
            mode='nearest')
    voxel[np.nonzero(voxel)] = 1.0
    return voxel

def read_binvox(path, shape=(64,64,64), fix_coords=True):
    """
    read voxel data from .binvox file
    """
    with open(path, 'rb') as f:
        voxel = binvox_rw.read_as_3d_array(f, fix_coords)
    
    voxel_data = voxel.data.astype(np.float)
    if shape is not None and voxel_data.shape != shape:
        voxel_data = resize(voxel.data.astype(np.float64), shape)

    return voxel_data

def write_binvox(data, path):
    """
    write out voxel data
    """
    data = np.rint(data).astype(np.uint8)
    dims = data.shape
    translate = [0., 0., 0.]
    scale = 1.0
    axis_order = 'xyz'
    v = binvox_rw.Voxels( data, dims, translate, scale, axis_order)

    with open(path, 'bw') as f:
        v.write(f)

def read_all_binvox(directory):
    """
    read all .binvox files in the direcotry
    """
    input_files = [f for f in glob.glob(directory + "/**/*.binvox", recursive=True)]

    data = np.array([read_binvox(path) for path in input_files])
    n, w, h, d = data.shape

    return data.reshape(n, w, h, d, 1)

# ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
def convert_h5(in_path, out_path):
    data = read_binvox(in_path)
    f = h5py.File(out_path, 'w')
    # more compression options: https://docs.h5py.org/en/stable/high/dataset.html
    f.create_dataset('data', data = data, compression='gzip')
    f.flush()
    f.close()

    return out_path

def check_open_h5(save_dir):
    if save_dir[-1] != '/':
        save_dir += '/'

    files = [f for f in glob.glob(save_dir + "**/*.h5", recursive=True)]
    for path in files:
        try:
            read_h5(path)
        except KeyError:
            print(path)

def extract_name(path):
    return path.split('/')[-1].split('.')[0]

def main():
    if len(sys.argv) < 3:
        print("usage: python dump.py dataset_path(.binvox) save_dir")
        sys.exit()
    
    dataset_path = sys.argv[1]
    save_dir = sys.argv[2]
    
    if dataset_path[-1] != '/':
        dataset_path += '/'
    if save_dir[-1] != '/':
        save_dir += '/'

    os.makedirs(save_dir, exist_ok=True)
    files = [f for f in glob.glob(dataset_path + "**/*.binvox", recursive=True)]
    print("{} files found.".format(len(files)))
    
    ext = sys.argv[1]
    for in_path in files:
        out_path = save_dir + extract_name(in_path) + '.h5'
        result = convert_h5(in_path, out_path)

if __name__=="__main__":
    # main()
    check_open_h5(sys.argv[1])
