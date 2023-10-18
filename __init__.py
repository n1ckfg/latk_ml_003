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

import os
import sys
import numpy as np
import latk
import latk_blender as lb
import random

import torch
import skeletor as sk
import trimesh
from scipy.spatial.distance import cdist
from scipy.spatial import Delaunay

def findAddonPath(name=None):
    if not name:
        name = __name__
    for mod in addon_utils.modules():
        if mod.bl_info["name"] == name:
            url = mod.__file__
            return os.path.dirname(url)
    return None

sys.path.append(os.path.join(findAddonPath(), "vox2vox"))


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
            ("STROKE_GEN", "Connect Strokes", "...", 0),
            ("CONTOUR_GEN", "Connect Contours", "...", 1),
            ("SKEL_GEN", "Connect Skeleton", "...", 2)
        ),
        default="STROKE_GEN"
    )

    Model: EnumProperty(
        name="Model",
        items=(
            ("256V001", "256x256 v001", "...", 0),
            ("256V002", "256x256 v002", "...", 1)
        ),
        default="256V001"
    )

    thickness: FloatProperty(
        name="Thickness %",
        description="...",
        default=10.0
    )

    strokegen_radius: FloatProperty(
        name="StrokeGen Radius",
        description="Base search distance for points",
        default=2
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
        net1, net2 = loadModel()

        la = latk.Latk()
        la.layers.append(latk.LatkLayer())

        start, end = lb.getStartEnd()
        for i in range(start, end):
            lb.goToFrame(i)
            laFrame = doInference(net1, net2)
            la.layers[0].frames.append(laFrame)

        lb.fromLatkToGp(la, resizeTimeline=False)
        lb.setThickness(latkml003.thickness)
        return {'FINISHED'}


class latkml003_Button_SingleFrame(bpy.types.Operator):
    """Operate on a single frame"""
    bl_idname = "latkml003_button.singleframe"
    bl_label = "Single Frame"
    bl_options = {'UNDO'}
    
    def execute(self, context):
        latkml003 = context.scene.latkml003_settings
        
        op1 = latkml003.Operation1.lower() 
        op2 = latkml003.Operation2.lower() 

        if (op1 == "voxel_ml"):
            net1, net2 = loadModel()

            la = latk.Latk()
            la.layers.append(latk.LatkLayer())
            laFrame = doInference(net1, net2)
            la.layers[0].frames.append(laFrame)
        
            lb.fromLatkToGp(la, resizeTimeline=False)
            lb.setThickness(latkml003.thickness)
            # automatically do connections in inference step
        else:
            if (op2 == "skel_gen"):
                skelGen()
            elif (op2 == "contour_gen"):
                contourGen()
            else:
                strokeGen(radius=latkml003.strokegen_radius, minPointsCount=latkml003.strokegen_minPointsCount, limitPalette=context.scene.latk_settings.paletteLimit)
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
        row.prop(latkml003, "thickness")

        row = layout.row()
        row.prop(latkml003, "strokegen_radius")
        row.prop(latkml003, "strokegen_minPointsCount")

        row = layout.row()

        row = layout.row()

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

def getModelPath(url):
    return os.path.join(findAddonPath(), url)

def loadModel():
    latkml003 = bpy.context.scene.latkml003_settings
    returns = modelSelector(latkml003.Model)
    return returns

def modelSelector(modelName):
    modelName = modelName.lower()
    
    if (modelName == "256v001"):
        return Vox2Vox_PyTorch("model/generator_100.pth")
    elif (modelName == "256v002"):
        return Vox2Vox_PyTorch("model/generator_100.pth")
    elif (modelName == "256v003"):
        return Vox2Vox_PyTorch("model/generator_100.pth")
    else:
        return None

def doInference(net):
    latkml003 = bpy.context.scene.latkml003_settings

def getPyTorchDevice():
    device = None
    if torch.cuda.is_available():
        device = torch.device("cuda")
    elif torch.backends.mps.is_available():
        device = torch.device("mps")
    else:
        device = torch.device("cpu")
        return device

def createPyTorchNetwork(modelPath, net_G, device, input_nc=3, output_nc=1, n_blocks=3):
    #device = getPyTorchDevice()
    modelPath = getModelPath(modelPath)
    net_G.to(device)
    net_G.load_state_dict(torch.load(modelPath, map_location=device))
    net_G.eval()
    return net_G


class Vox2Vox_PyTorch():
    def __init__(self, modelPath):
        self.device = getPyTorchDevice()         
        generator = Generator(3, 1, 3) # input_nc=3, output_nc=1, n_blocks=3
        self.net_G = createPyTorchNetwork(modelPath, generator, self.device)   

    def detect(self, srcimg):
        with torch.no_grad():   
            srcimg2 = np.transpose(srcimg, (2, 0, 1))

            tensor_array = torch.from_numpy(srcimg2)
            input_tensor = tensor_array.to(self.device)
            output_tensor = self.net_G(input_tensor)

            result = output_tensor.detach().cpu().numpy().transpose(1, 2, 0)
            result *= 255
            result = cv2.resize(result, (srcimg.shape[1], srcimg.shape[0]))
            
            return result

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

def strokeGen(obj=None, radius=2, minPointsCount=5, limitPalette=32):
    origCursorLocation = bpy.context.scene.cursor.location
    bpy.context.scene.cursor.location = (0.0, 0.0, 0.0)

    if not obj:
        obj = lb.ss()
    mesh = obj.data
    #matrixWorld = obj.matrix_world
    #matrixWorldInverted = matrixWorld.inverted()
    #~
    gp = lb.getActiveGp()
    layer = lb.getActiveLayer()
    if not layer:
        layer = gp.data.layers.new(name="meshToGp")
    frame = lb.getActiveFrame()
    if not frame or frame.frame_number != lb.currentFrame():
        frame = layer.frames.new(lb.currentFrame())
    #~
    images = None
    try:
        images = lb.getUvImages()
    except:
        pass
    #~
    allPoints, allColors = lb.getVertsAndColors(target=obj, useWorldSpace=True, useColors=True, useBmesh=False)
    #~
    strokeGroups = group_points_into_strokes(allPoints, radius, minPointsCount)

    for i, strokeGroup in enumerate(strokeGroups):
        colorIndex = strokeGroup[0]

        color = (1, 1, 1)

        if not images:
            try:
                color = allColors[colorIndex]
            except:
                try:
                    color = lb.getColorExplorer(obj, colorIndex)
                except:
                    color = (1, 1, 1)
        else:
            try:
                color = lb.getColorExplorer(obj, colorIndex, images)
            except:
                color = lb.getColorExplorer(obj, colorIndex)
        color = (color[0], color[1], color[2])

        if (limitPalette == 0):
            lb.createColor(color)
        else:
            lb.createAndMatchColorPalette(color, limitPalette, 5) # num places
        
        stroke = frame.strokes.new()
        stroke.display_mode = '3DSPACE'
        stroke.line_width = 10 # adjusted from 100 for 2.93
        stroke.material_index = gp.active_material_index

        stroke.points.add(len(strokeGroup))

        for j, index in enumerate(strokeGroup):    
            point = allPoints[index]
            #point = matrixWorldInverted @ mathutils.Vector((point[0], point[2], point[1]))
            #point = (point[0], point[1], point[2])
            pressure = 1.0
            strength = 1.0
            lb.createPoint(stroke, j, point, pressure, strength)

    bpy.context.scene.cursor.location = origCursorLocation

def contourGen(obj=None):
    origCursorLocation = bpy.context.scene.cursor.location
    bpy.context.scene.cursor.location = (0.0, 0.0, 0.0)

    la = latk.Latk(init=True)

    if not obj:
        obj = lb.ss()
    #mesh = obj.data
    matrixWorld = obj.matrix_world
    #matrixWorldInverted = matrixWorld.inverted()

    verts = lb.getVertices(obj)
    faces = lb.getFaces(obj)
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
                la_s = latk.LatkStroke()

                for index in entity.points:
                    vert = slice_mesh.vertices[index] 
                    vert = matrixWorld @ mathutils.Vector(vert)
                    #vert = [vert[0], vert[1], vert[2]]
                    la_p = latk.LatkPoint(co=vert)
                    la_s.points.append(la_p)

                la.layers[0].frames[0].strokes.append(la_s)

    lb.fromLatkToGp(la)

    bpy.context.scene.cursor.location = origCursorLocation

def skelGen(obj=None):
    origCursorLocation = bpy.context.scene.cursor.location
    bpy.context.scene.cursor.location = (0.0, 0.0, 0.0)

    la = latk.Latk(init=True)

    if not obj:
        obj = lb.ss()
    matrixWorld = obj.matrix_world
    #matrixWorldInverted = matrixWorld.inverted()

    verts = lb.getVertices(obj)
    faces = lb.getFaces(obj)

    mesh = trimesh.Trimesh(verts, faces)

    fixed = sk.pre.fix_mesh(mesh, remove_disconnected=5, inplace=False)
    skel = sk.skeletonize.by_wavefront(fixed, waves=1, step_size=1)

    for entity in skel.skeleton.entities:
        la_s = latk.LatkStroke()

        for index in entity.points:
            vert = skel.vertices[index]
            vert = matrixWorld @ mathutils.Vector((vert[0], vert[1], vert[2]))
            la_p = latk.LatkPoint(co=vert)
            la_s.points.append(la_p)

        la.layers[0].frames[0].strokes.append(la_s)

    lb.fromLatkToGp(la)

    bpy.context.scene.cursor.location = origCursorLocation
