from torch.utils.data import Dataset, DataLoader
import h5py
import numpy as np
import glob

import binvox_rw

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

def read_binvox(path, shape=(128,128,128), fix_coords=True):
    """
    read voxel data from .binvox file
    """
    with open(path, 'rb') as f:
        voxel = binvox_rw.read_as_3d_array(f, fix_coords)
    
    voxel_data = voxel.data.astype(np.float)
    if shape is not None and voxel_data.shape != shape:
        voxel_data = resize(voxel.data.astype(np.float64), shape)

    return voxel_data

class CTDataset(Dataset):
    def __init__(self, datapath, transforms_):
        self.datapath = datapath
        self.transforms = transforms_
        #self.samples = ['../..'+x.split('.')[4] for x in glob.glob(self.datapath + '/*.im')]
        self.samples = [x.split('.')[0] for x in glob.glob(self.datapath + '/*.binvox')]

    def __len__(self):
        return len(self.samples)

    def __getitem__(self, idx):
        # print(self.samples[idx])
        data = read_binvox(self.samples[idx] + '.binvox')
        image = h5py.File(out_path, 'w')
        image.create_dataset('data', data = data, compression='gzip')
        #image = h5py.File(self.samples[idx] + '.im', 'r').get('data')[()]
        #mask = h5py.File(self.samples[idx] + '.seg', 'r').get('data')[()]
        # print(self.samples[idx])
        # print(image.shape)
        # print(mask.shape)
        if self.transforms:
            image = self.transforms(image)
            #mask = self.transforms(mask)
        
        return {"A": image, "B": image} #mask}

