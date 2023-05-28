from PIL import Image
import numpy as np 
import os
import argparse
import errno
import random

NB_VIEWPOINTS=8
LOG=True

def black_mask():
    return Image.fromarray(np.zeros([256,256])).convert("L")

def make_folder_safe(path):
    try:    
        os.makedirs(path)
    except:
        pass  

def make_hierarchy(path):
    try:    
        os.makedirs(path)
    except OSError as e:
        if e.errno == errno.EEXIST:
            print "Warning : out folder already exists."
        else:
            pass

    make_folder_safe(path+"/train")
    make_folder_safe(path+"/test")
    make_folder_safe(path+"/val")
    make_folder_safe(path+"/mask")
    make_folder_safe(path+"/mask/train")
    make_folder_safe(path+"/mask/test")
    make_folder_safe(path+"/mask/val")

def do(args,n,v,folder,mask):
    if not os.path.isfile(args.dataset_dir+"/%s/%s_%s.png"%(folder, n, v)):
        # print "Missing %s _ %s" %(n,v)
        try:
            sketch = np.array(Image.open(args.contours_dir+"/%s_%s.png"%(n,v)).convert('RGB'))
        except:
            print "missing sketch"
            return

        try:
            normal = np.array(Image.open(args.normals_dir+"/%s_%s.png"%(n,v)).convert("RGB"))
        except:
            print "missing normal"
            return

        pix = np.hstack( np.asarray(i) for i in [sketch, normal] )
        img = Image.fromarray(pix)

        img.save(args.dataset_dir+"/%s/%s_%s.png"%(folder, n, v))
        mask.save(args.dataset_dir+"/mask/%s/%s_%s.png"%(folder, n, v))
    # else:
    #     print "Skipping %s _ %s" %(n,v)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()    
    parser.add_argument("-contours_dir", help="folder for sketches (png)", type=str, required=True)
    parser.add_argument("-normals_dir", help="folder for normals (png)", type=str, required=True)
    parser.add_argument("-dataset_dir", help="output dataset folder", type=str, required=True)
    parser.add_argument("-start", help="start value", type=int, default=0)
    parser.add_argument("-nb", help="nb of images to do in this batch", type=int, default=20000)
    parser.add_argument("-nb_data", help="total number of images to use", type=int, default=20000)

    args = parser.parse_args()

    make_hierarchy(args.dataset_dir)

    mask = black_mask()    

    nb_train = int(0.80 * args.nb_data)
    nb_test = int(0.15 * args.nb_data)
    nb_val = int(0.05 * args.nb_data)

    nb = 0
    print "train: %s / test: %s / val: %s" % (nb_train, nb_test, nb_val)

    for n in range(args.start, args.start + args.nb):
        if n < nb_train:
            if LOG: print "train ", str(n)
            for v in range(NB_VIEWPOINTS):
                do(args, n, v, 'train', mask)
                nb = nb + 1   
                
        elif n < nb_test+nb_train:  
            if LOG: print "test ", str(n)
            for v in range(NB_VIEWPOINTS):
                do(args, n, v, 'test', mask)
                nb = nb + 1

        elif n < nb_val+nb_test+nb_train:
            if LOG: print "val ", str(n)        
            for v in range(NB_VIEWPOINTS):
                do(args, n, v, 'val', mask)
                nb = nb + 1            

    print "Number of entries copied : " + str(nb)