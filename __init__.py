bl_info = {
    "name": "latk_ml_003", 
    "author": "Nick Fox-Gieg",
    "version": (0, 0, 1),
    "blender": (3, 0, 0),
    "description": "Generate brushstrokes from a mesh using vox2vox",
    "category": "Animation"
}

import bpy
import gpu
import bgl
from bpy.types import Operator, AddonPreferences
from bpy.props import (BoolProperty, FloatProperty, StringProperty, IntProperty, PointerProperty, EnumProperty)
from bpy_extras.io_utils import (ImportHelper, ExportHelper)
import addon_utils
import mathutils
import h5py

import os
import sys
import numpy as np
import latk
import latk_blender as lb
import random

import torch
from torch.autograd import Variable
import torchvision.transforms as transforms
from torch.utils.data import DataLoader
import itertools
from torchvision import datasets

import skeletor as sk
import trimesh
from scipy.spatial.distance import cdist
from scipy.spatial import Delaunay
import scipy.ndimage as nd
from pyntcloud import PyntCloud 
import pandas as pd
import pdb

def findAddonPath(name=None):
    if not name:
        name = __name__
    for mod in addon_utils.modules():
        if mod.bl_info["name"] == name:
            url = mod.__file__
            return os.path.dirname(url)
    return None

from . import binvox_rw
from .vox2vox.models import *
from .vox2vox.dataset import CTDataset

class latkml003Preferences(bpy.types.AddonPreferences):
    bl_idname = __name__

    '''
    extraFormats_AfterEffects: bpy.props.BoolProperty(
        name = 'After Effects JSX',
        description = "After Effects JSX export",
        default = False
    )
    '''

    def draw(self, context):
        layout = self.layout

        layout.label(text="none")
        #row = layout.row()
        #row.prop(self, "extraFormats_Painter")


# This is needed to display the preferences menu
# https://docs.blender.org/api/current/bpy.types.AddonPreferences.html
class OBJECT_OT_latkml003_prefs(Operator):
    """Display example preferences"""
    bl_idname = "object.latkml003" #+ __name__
    bl_label = "latkml003 Preferences"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        preferences = context.preferences
        addon_prefs = preferences.addons[__name__].preferences
        return {'FINISHED'}


class latkml003Properties(bpy.types.PropertyGroup):
    """Properties for latkml003"""
    bl_idname = "GREASE_PENCIL_PT_latkml003Properties"

    Operation1: EnumProperty(
        name="Operation 1",
        items=(
            ("NONE", "None", "...", 0),
            ("VOXEL_ML", "Voxelize", "...", 1)
        ),
        default="NONE"
    )

    Operation2: EnumProperty(
        name="Operation 2",
        items=(
            ("NONE", "None", "...", 0),
            ("GET_EDGES", "Get Edges", "...", 1)
        ),
        default="NONE"
    )

    Operation3: EnumProperty(
        name="Operation 3",
        items=(
            ("STROKE_GEN", "Connect Strokes", "...", 0),
            ("CONTOUR_GEN", "Connect Contours", "...", 1),
            ("SKEL_GEN", "Connect Skeleton", "...", 2)
        ),
        default="STROKE_GEN"
    )

    Model: EnumProperty(
        name="Model",
        items=(
            ("256_V008", "256 v008", "...", 0),
            ("128_V008", "128 v008", "...", 1)
        ),
        default="256_V008"
    )

    thickness: FloatProperty(
        name="Thickness %",
        description="...",
        default=10.0
    )

    dims: IntProperty(
        name="Dims",
        description="Voxel Dimensions",
        default=256
    )

    strokegen_radius: FloatProperty(
        name="StrokeGen Radius",
        description="Base search distance for points",
        default=1
    )

    strokegen_minPointsCount: IntProperty(
        name="StrokeGen Min Points",
        description="Minimum number of points to make a stroke",
        default=5
    )

class latkml003_Button_AllFrames(bpy.types.Operator):
    """Operate on all frames"""
    bl_idname = "latkml003_button.allframes"
    bl_label = "All Frames"
    bl_options = {'UNDO'}
    
    def execute(self, context):
        latkml003 = context.scene.latkml003_settings
        dims = latkml003.dims
        
        op1 = latkml003.Operation1.lower() 
        op2 = latkml003.Operation2.lower() 
        op3 = latkml003.Operation3.lower() 

        net1 = None
        obj = lb.ss()

        for i in range(start, end):
            lb.goToFrame(i)

            verts_alt, colors = lb.getVertsAndColors(target=obj, useWorldSpace=False, useColors=True, useBmesh=False)
            verts = lb.getVertices(obj)
            faces = lb.getFaces(obj)
            matrix_world = obj.matrix_world
            bounds = obj.dimensions
            avgBounds = (bounds.x + bounds.y + bounds.z) / 3.0

            if (op1 == "voxel_ml"):
                if not net1:
                    net1 = loadModel()           
                verts = doInference(net1, verts, dims, bounds)

            if (op2 == "get_edges"):
                verts = differenceEigenvalues(verts)

            if (op3 == "skel_gen" and op1 == op2):
                skelGen(verts, faces, matrix_world)
            elif (op3 == "contour_gen" and op1 == op2):
                contourGen(verts, faces, matrix_world)
            else:
                strokeGen(verts, colors, matrix_world, radius=avgBounds * latkml003.strokegen_radius, minPointsCount=latkml003.strokegen_minPointsCount, limitPalette=context.scene.latk_settings.paletteLimit)
            
        return {'FINISHED'}


class latkml003_Button_SingleFrame(bpy.types.Operator):
    """Operate on a single frame"""
    bl_idname = "latkml003_button.singleframe"
    bl_label = "Single Frame"
    bl_options = {'UNDO'}
    
    def execute(self, context):
        latkml003 = context.scene.latkml003_settings
        dims = latkml003.dims
        
        op1 = latkml003.Operation1.lower() 
        op2 = latkml003.Operation2.lower() 
        op3 = latkml003.Operation3.lower() 

        net1 = None
        obj = lb.ss()

        verts_alt, colors = lb.getVertsAndColors(target=obj, useWorldSpace=False, useColors=True, useBmesh=False)
        verts = lb.getVertices(obj)
        faces = lb.getFaces(obj)
        matrix_world = obj.matrix_world
        bounds = obj.dimensions
        avgBounds = (bounds.x + bounds.y + bounds.z) / 3.0

        if (op1 == "voxel_ml"):
            if not net1:
                net1 = loadModel()           
            verts = doInference(net1, verts, dims, bounds)

        if (op2 == "get_edges"):
            verts = differenceEigenvalues(verts)

        if (op3 == "skel_gen" and op1 == op2):
            skelGen(verts, faces, matrix_world)
        elif (op3 == "contour_gen" and op1 == op2):
            contourGen(verts, faces, matrix_world)
        else:
            strokeGen(verts, colors, matrix_world, radius=avgBounds * latkml003.strokegen_radius, minPointsCount=latkml003.strokegen_minPointsCount, limitPalette=context.scene.latk_settings.paletteLimit)
        
        return {'FINISHED'}


# https://blender.stackexchange.com/questions/167862/how-to-create-a-button-on-the-n-panel
class latkml003Properties_Panel(bpy.types.Panel):
    """Creates a Panel in the 3D View context"""
    bl_idname = "GREASE_PENCIL_PT_latkml003PropertiesPanel"
    bl_space_type = 'VIEW_3D'
    bl_label = "latk_ml_003"
    bl_category = "Latk"
    bl_region_type = 'UI'
    #bl_context = "objectmode" # "mesh_edit"

    def draw(self, context):
        layout = self.layout

        scene = context.scene
        latkml003 = scene.latkml003_settings

        row = layout.row()
        row.operator("latkml003_button.singleframe")
        row.operator("latkml003_button.allframes")

        row = layout.row()
        row.prop(latkml003, "Operation1")
        row = layout.row()
        row.prop(latkml003, "Model")

        row = layout.row()
        row.prop(latkml003, "Operation2")

        row = layout.row()
        row.prop(latkml003, "Operation3")
        row = layout.row()
        row.prop(latkml003, "thickness")
        row = layout.row()
        row.prop(latkml003, "strokegen_radius")
        row.prop(latkml003, "strokegen_minPointsCount")


classes = (
    OBJECT_OT_latkml003_prefs,
    latkml003Preferences,
    latkml003Properties,
    latkml003Properties_Panel,
    latkml003_Button_AllFrames,
    latkml003_Button_SingleFrame
)

def register():
    for cls in classes:
        bpy.utils.register_class(cls)   
    bpy.types.Scene.latkml003_settings = bpy.props.PointerProperty(type=latkml003Properties)

def unregister():
    del bpy.types.Scene.latkml003_settings
    for cls in reversed(classes):
        bpy.utils.unregister_class(cls)

if __name__ == "__main__":
    register()

def remap(value, min1, max1, min2, max2):
    return np.interp(value,[min1, max1],[min2, max2])

def normalize(verts, minVal=0.0, maxVal=1.0):
    newVerts = []
    allX = []
    allY = []
    allZ = []

    for vert in verts:       
        allX.append(vert[0])
        allY.append(vert[1])
        allZ.append(vert[2])
    
    allX.sort()
    allY.sort()
    allZ.sort()
    
    leastValArray = [ allX[0], allY[0], allZ[0] ]
    mostValArray = [ allX[len(allX)-1], allY[len(allY)-1], allZ[len(allZ)-1] ]
    leastValArray.sort()
    mostValArray.sort()
    leastVal = leastValArray[0]
    mostVal = mostValArray[2]
    valRange = mostVal - leastVal
    
    xRange = (allX[len(allX)-1] - allX[0]) / valRange
    yRange = (allY[len(allY)-1] - allY[0]) / valRange
    zRange = (allZ[len(allZ)-1] - allZ[0]) / valRange
    
    minValX = minVal * xRange
    minValY = minVal * yRange
    minValZ = minVal * zRange
    maxValX = maxVal * xRange
    maxValY = maxVal * yRange
    maxValZ = maxVal * zRange
    
    for vert in verts:
        x = remap(vert[0], allX[0], allX[len(allX)-1], minValX, maxValX)
        y = remap(vert[1], allY[0], allY[len(allY)-1], minValY, maxValY)
        z = remap(vert[2], allZ[0], allZ[len(allZ)-1], minValZ, maxValZ)
        newVerts.append((x,y,z))

    return newVerts

def scale_numpy_array(arr, min_v, max_v):
    new_range = (min_v, max_v)
    max_range = max(new_range)
    min_range = min(new_range)

    scaled_unit = (max_range - min_range) / (np.max(arr) - np.min(arr))
    return arr * scaled_unit - np.min(arr) * scaled_unit + min_range

def resizeVoxels(voxel, shape):
    ratio = shape[0] / voxel.shape[0]
    voxel = nd.zoom(voxel,
            ratio,
            order=1, 
            mode='nearest')
    voxel[np.nonzero(voxel)] = 1.0
    return voxel

def vertsToBinvox(verts, dims=256, doFilter=False, axis='xyz'):
    shape = (dims, dims, dims)
    data = np.zeros(shape, dtype=bool)
    translate = (0, 0, 0)
    scale = 1
    axis_order = axis
    bv = binvox_rw.Voxels(data, shape, translate, scale, axis_order)

    verts = scale_numpy_array(verts, 0, dims - 1)

    for vert in verts:
        x = dims - 1 - int(vert[0])
        y = int(vert[1])
        z = int(vert[2])
        data[x][y][z] = True

    if (doFilter == True):
        for i in range(0, 1): # 1
            nd.binary_dilation(bv.data.copy(), output=bv.data)

        for i in range(0, 3): # 3
            nd.sobel(bv.data.copy(), output=bv.data)

        nd.median_filter(bv.data.copy(), size=4, output=bv.data) # 4

        for i in range(0, 2): # 2
            nd.laplace(bv.data.copy(), output=bv.data)

        for i in range(0, 0): # 0
            nd.binary_erosion(bv.data.copy(), output=bv.data)

    return bv

def binvoxToVerts(voxel, dims=256, axis='xyz'):
    verts = []
    for x in range(0, dims):
        for y in range(0, dims):
            for z in range(0, dims):
                if (voxel.data[x][y][z] == True):
                    verts.append([dims-1-z, y, x])
    return verts

def binvoxToH5(voxel, dims=256):
    shape=(dims, dims, dims)   
    voxel_data = voxel.data.astype(float) #voxel.data.astype(np.float)
    if shape is not None and voxel_data.shape != shape:
        voxel_data = resize(voxel.data.astype(np.float64), shape)
    return voxel_data

def h5ToBinvox(data, dims=256):
    data = np.rint(data).astype(np.uint8)
    shape = (dims, dims, dims) #data.shape
    translate = [0, 0, 0]
    scale = 1.0
    axis_order = 'xzy'
    return binvox_rw.Voxels(data, shape, translate, scale, axis_order)

def writeTempH5(data):
    url = os.path.join(bpy.app.tempdir, "output.im")
    f = h5py.File(url, 'w')
    # more compression options: https://docs.h5py.org/en/stable/high/dataset.html
    f.create_dataset('data', data=data, compression='gzip')
    f.flush()
    f.close()

def readTempH5():
    url = os.path.join(bpy.app.tempdir, "output.im")
    return h5py.File(url, 'r').get('data')[()]


def writeTempBinvox(data, dims=256):
    url = os.path.join(bpy.app.tempdir, "output.binvox")
    data = np.rint(data).astype(np.uint8)
    shape = (dims, dims, dims) #data.shape
    translate = [0, 0, 0]
    scale = 1.0
    axis_order = 'xzy'
    voxel = binvox_rw.Voxels(data, shape, translate, scale, axis_order)

    with open(url, 'bw') as f:
        voxel.write(f)

def readTempBinvox(dims=256, axis='xyz'):
    url = os.path.join(bpy.app.tempdir, "output.binvox")
    voxel = None
    print("Reading from: " + url)
    with open(url, 'rb') as f:
        voxel = binvox_rw.read_as_3d_array(f, True) # fix coords
    verts = []
    for x in range(0, dims):
        for y in range(0, dims):
            for z in range(0, dims):
                if (voxel.data[x][y][z] == True):
                    verts.append([dims-1-z, y, x])
    return verts

def getModelPath(url):
    return os.path.join(findAddonPath(), url)

def loadModel():
    latkml003 = bpy.context.scene.latkml003_settings
    returns = modelSelector(latkml003.Model)
    return returns

def modelSelector(modelName):
    latkml003 = bpy.context.scene.latkml003_settings

    modelName = modelName.lower()
    latkml003.dims = int(modelName.split("_")[0])

    if (modelName == "256_v008"):
        latkml003.dims = 256
        return Vox2Vox_PyTorch("model/256_100.pth")
    elif (modelName == "128_v008"):
        latkml003.dims = 128
        return Vox2Vox_PyTorch("model/128_100.pth")
    else:
        return None

def getPyTorchDevice():
    device = None
    if torch.cuda.is_available():
        device = torch.device("cuda")
    #elif torch.backends.mps.is_available():
        #device = torch.device("mps")
    else:
        device = torch.device("cpu")
    return device

def createPyTorchNetwork(modelPath, net_G, device): #, input_nc=3, output_nc=1, n_blocks=3):
    modelPath = getModelPath(modelPath)
    net_G.to(device)
    net_G.load_state_dict(torch.load(modelPath, map_location=device))
    net_G.eval()
    return net_G

def doInference(net, verts, dims=256, bounds=(1,1,1)):
    bv = vertsToBinvox(verts, dims=dims, doFilter=True)
    h5 = binvoxToH5(bv, dims=dims)
    writeTempH5(h5)

    fake_B = net.detect()

    writeTempBinvox(fake_B, dims=dims)
    verts = readTempBinvox(dims=dims)
    newVerts = normalize(verts)

    dims_ = float(dims-1)
    for i in range(0, len(newVerts)):
        vert = newVerts[i]
        x = vert[0] * bounds.x
        y = vert[1] * bounds.y
        z = vert[2] * bounds.z
        newVerts[i] = (x, y, z)

    return newVerts


class Vox2Vox_PyTorch():
    def __init__(self, modelPath):
        self.device = getPyTorchDevice()         
        generator = GeneratorUNet()
        if self.device.type == "cuda":
            generator = generator.cuda()

        self.net_G = createPyTorchNetwork(modelPath, generator, self.device)

        self.transforms_ = transforms.Compose([
            transforms.ToTensor()
        ])

    def detect(self):
        Tensor = None
        if self.device.type == "cuda":
            Tensor = torch.cuda.FloatTensor
        else:
            Tensor = torch.FloatTensor

        val_dataloader = DataLoader(
            CTDataset(bpy.app.tempdir, transforms_=self.transforms_, isTest=True),
            batch_size=1,
            shuffle=False,
            num_workers=0,
        )

        dataiter = iter(val_dataloader)
        imgs = next(dataiter) #dataiter.next()

        """Saves a generated sample from the validation set"""
        real_A = Variable(imgs["A"].unsqueeze_(1).type(Tensor))
        #real_B = Variable(imgs["B"].unsqueeze_(1).type(Tensor))
        fake_B = self.net_G(real_A)

        return fake_B.cpu().detach().numpy()


def group_points_into_strokes(points, radius, minPointsCount):
    strokeGroups = []
    unassigned_points = set(range(len(points)))

    while len(unassigned_points) > 0:
        strokeGroup = [next(iter(unassigned_points))]
        unassigned_points.remove(strokeGroup[0])

        for i in range(len(points)):
            if i in unassigned_points and cdist([points[i]], [points[strokeGroup[-1]]])[0][0] < radius:
                strokeGroup.append(i)
                unassigned_points.remove(i)

        if (len(strokeGroup) >= minPointsCount):
            strokeGroups.append(strokeGroup)

        print("Found " + str(len(strokeGroups)) + " strokeGroups, " + str(len(unassigned_points)) + " points remaining.")
    return strokeGroups

def strokeGen(verts, colors, matrix_world, radius=2, minPointsCount=5, limitPalette=32):
    latkml003 = bpy.context.scene.latkml003_settings
    origCursorLocation = bpy.context.scene.cursor.location
    bpy.context.scene.cursor.location = (0.0, 0.0, 0.0)

    gp = lb.getActiveGp()
    layer = lb.getActiveLayer()
    if not layer:
        layer = gp.data.layers.new(name="meshToGp")
    frame = lb.getActiveFrame()
    if not frame or frame.frame_number != lb.currentFrame():
        frame = layer.frames.new(lb.currentFrame())

    strokeGroups = group_points_into_strokes(verts, radius, minPointsCount)

    for i, strokeGroup in enumerate(strokeGroups):
        colorIndex = strokeGroup[0]
        color = None

        if not colors:
            color = (1, 1, 1)
        else:    
            color = colors[colorIndex]

        if (limitPalette == 0):
            lb.createColor(color)
        else:
            lb.createAndMatchColorPalette(color, limitPalette, 5) # num places
        
        stroke = frame.strokes.new()
        stroke.display_mode = '3DSPACE'
        stroke.line_width = int(latkml003.thickness) #10 # adjusted from 100 for 2.93
        stroke.material_index = gp.active_material_index

        stroke.points.add(len(strokeGroup))

        for j, index in enumerate(strokeGroup):    
            point = matrix_world @ mathutils.Vector(verts[index])
            #point = matrixWorldInverted @ mathutils.Vector((point[0], point[2], point[1]))
            #point = (point[0], point[1], point[2])
            pressure = 1.0
            strength = 1.0
            lb.createPoint(stroke, j, point, pressure, strength)

    bpy.context.scene.cursor.location = origCursorLocation

def contourGen(verts, faces, matrix_world):
    latkml003 = bpy.context.scene.latkml003_settings
    origCursorLocation = bpy.context.scene.cursor.location
    bpy.context.scene.cursor.location = (0.0, 0.0, 0.0)

    la = latk.Latk(init=True)

    gp = lb.getActiveGp()
    layer = lb.getActiveLayer()
    if not layer:
        layer = gp.data.layers.new(name="meshToGp")
    frame = lb.getActiveFrame()
    if not frame or frame.frame_number != lb.currentFrame():
        frame = layer.frames.new(lb.currentFrame())

    mesh = None

    if len(faces) < 1:
        tri = Delaunay(verts)
        mesh = trimesh.Trimesh(tri.points, tri.simplices)
    else:
        mesh = trimesh.Trimesh(verts, faces)

    bounds = lb.getDistance(mesh.bounds[0], mesh.bounds[1])

    # generate a set of contour lines at regular intervals
    interval = bounds * 0.01 #0.03  #0.1 # the spacing between contours
    print("Interval: " + str(interval))

    # x, z
    slice_range = np.arange(mesh.bounds[0][2], mesh.bounds[1][2], interval)
    # y
    #slice_range = np.arange(mesh.bounds[0][1], mesh.bounds[0][2], interval)

    # loop over the z values and generate a contour at each level
    for slice_pos in slice_range:
        # x
        #slice_mesh = mesh.section(plane_origin=[slice_pos, 0, 0], plane_normal=[1, 0, 0])
        # y
        #slice_mesh = mesh.section(plane_origin=[0, slice_pos, 0], plane_normal=[0, 1, 0])
        # z
        slice_mesh = mesh.section(plane_origin=[0, 0, slice_pos], plane_normal=[0, 0, 1])
        
        if slice_mesh != None:
            for entity in slice_mesh.entities:
                stroke = frame.strokes.new()
                stroke.display_mode = '3DSPACE'
                stroke.line_width = int(latkml003.thickness) #10 # adjusted from 100 for 2.93
                stroke.material_index = gp.active_material_index
                stroke.points.add(len(entity.points))

                for i, index in enumerate(entity.points):
                    vert = slice_mesh.vertices[index] 
                    vert = matrix_world @ mathutils.Vector(vert)
                    #vert = [vert[0], vert[1], vert[2]]
                    lb.createPoint(stroke, i, vert, 1.0, 1.0)

    #lb.fromLatkToGp(la, resizeTimeline=False)
    #lb.setThickness(latkml003.thickness)

    bpy.context.scene.cursor.location = origCursorLocation

def skelGen(verts, faces, matrix_world):
    latkml003 = bpy.context.scene.latkml003_settings
    origCursorLocation = bpy.context.scene.cursor.location
    bpy.context.scene.cursor.location = (0.0, 0.0, 0.0)

    la = latk.Latk(init=True)

    gp = lb.getActiveGp()
    layer = lb.getActiveLayer()
    if not layer:
        layer = gp.data.layers.new(name="meshToGp")
    frame = lb.getActiveFrame()
    if not frame or frame.frame_number != lb.currentFrame():
        frame = layer.frames.new(lb.currentFrame())

    mesh = trimesh.Trimesh(verts, faces)

    fixed = sk.pre.fix_mesh(mesh, remove_disconnected=5, inplace=False)
    skel = sk.skeletonize.by_wavefront(fixed, waves=1, step_size=1)

    for entity in skel.skeleton.entities:
        stroke = frame.strokes.new()
        stroke.display_mode = '3DSPACE'
        stroke.line_width = int(latkml003.thickness) #10 # adjusted from 100 for 2.93
        stroke.material_index = gp.active_material_index
        stroke.points.add(len(entity.points))

        for i, index in enumerate(entity.points):
            vert = skel.vertices[index]
            vert = matrix_world @ mathutils.Vector((vert[0], vert[1], vert[2]))
            lb.createPoint(stroke, i, vert, 1.0, 1.0)

    #lb.fromLatkToGp(la, resizeTimeline=False)
    #lb.setThickness(latkml003.thickness)

    bpy.context.scene.cursor.location = origCursorLocation

def differenceEigenvalues(verts):
    # MIT License Copyright (c) 2015 Dena Bazazian Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions: The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

    pdVerts = pd.DataFrame(verts, columns=["x", "y", "z"])
    pcd1 = PyntCloud(pdVerts)
        
    # define hyperparameters
    k_n = 50 # 50
    thresh = 0.03 # 0.03

    pcd_np = np.zeros((len(pcd1.points),6))

    # find neighbors
    kdtree_id = pcd1.add_structure("kdtree")
    k_neighbors = pcd1.get_neighbors(k=k_n, kdtree=kdtree_id) 

    # calculate eigenvalues
    ev = pcd1.add_scalar_field("eigen_values", k_neighbors=k_neighbors)

    x = pcd1.points['x'].values 
    y = pcd1.points['y'].values 
    z = pcd1.points['z'].values 

    e1 = pcd1.points['e3('+str(k_n+1)+')'].values
    e2 = pcd1.points['e2('+str(k_n+1)+')'].values
    e3 = pcd1.points['e1('+str(k_n+1)+')'].values

    sum_eg = np.add(np.add(e1,e2),e3)
    sigma = np.divide(e1,sum_eg)
    sigma_value = sigma

    # visualize the edges
    sigma = sigma>thresh

    # Save the edges and point cloud
    thresh_min = sigma_value < thresh
    sigma_value[thresh_min] = 0
    thresh_max = sigma_value > thresh
    sigma_value[thresh_max] = 255

    pcd_np[:,0] = x
    pcd_np[:,1] = y
    pcd_np[:,2] = z
    pcd_np[:,3] = sigma_value

    edge_np = np.delete(pcd_np, np.where(pcd_np[:,3] == 0), axis=0) 
    print(len(edge_np))

    clmns = ['x','y','z','red','green','blue']
    #pcd_pd = pd.DataFrame(data=pcd_np,columns=clmns)
    #pcd_pd['red'] = sigma_value.astype(np.uint8)

    #pcd_points = PyntCloud(pcd_pd)
    #edge_points = PyntCloud(pd.DataFrame(data=edge_np,columns=clmns))

    #PyntCloud.to_file(edge_points, outputPath) # Save just the edge points
    newVerts = []
    #for i in range(0, len(edge_points.points)):
    #    newVerts.append((edge_points.points["x"][i], edge_points.points["y"][i], edge_points.points["z"][i]))
    for edge in edge_np:
        newVerts.append((edge[0], edge[1], edge[2]))

    return newVerts

