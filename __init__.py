bl_info = {
    "name": "latk_ml_003", 
    "author": "Nick Fox-Gieg",
	"version": (0, 0, 1),
	"blender": (3, 0, 0),
    "description": "Generate brushstrokes from a mesh using vox2vox",
    "category": "Animation"
}

import bpy
from bpy.types import Operator, AddonPreferences
from bpy.props import (BoolProperty, FloatProperty, StringProperty, IntProperty, PointerProperty, EnumProperty)
from bpy_extras.io_utils import (ImportHelper, ExportHelper)

import numpy as np
import latk
import latk_blender as lb
import random

import torch


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

    '''
    bakeMesh: BoolProperty(
        name="Bake",
        description="Off: major speedup if you're staying in Blender. On: slower but keeps everything exportable",
        default=True
    )
	'''

    latkml003_Model: EnumProperty(
        name="Model",
        items=(
            ("256V001", "256x256 v001", "...", 0),
            ("256V002", "256x256 v002", "...", 1)
        ),
        default="256V001"
    )

    latkml003_thickness: FloatProperty(
        name="Thickness %",
        description="...",
        default=10.0
    )

    strokegen1_strokeLength: IntProperty(
        name="Length",
        description="Group every n points into strokes",
        default=10
    )

    strokegen1_strokeGaps: FloatProperty(
        name="Gaps",
        description="Skip points greater than this distance away",
        default=10.0
    )

    strokegen1_shuffleOdds: FloatProperty(
        name="Odds",
        description="Odds of shuffling the points in a stroke",
        default=1.0
    )

    strokegen1_spreadPoints: FloatProperty(
        name="Spread",
        description="Distance to randomize points",
        default=0.1
    )

    strokegen2_radius: FloatProperty(
        name="Radius",
        description="Base search distance for points",
        default=2
    )

    strokegen2_minPointsCount: IntProperty(
        name="Min Points Count",
        description="Minimum number of points to make a stroke",
        default=10
    )


class latkml003_Button_StrokeGen1(bpy.types.Operator):
    """Generate GP strokes from a mesh"""
    bl_idname = "latkml003_button.strokegen1"
    bl_label = "StrokeGen 1"
    bl_options = {'UNDO'}
    
    def execute(self, context):
        latkml003 = context.scene.latkml003_settings
        latk = context.scene.latk_settings
        strokeGen1(strokeLength=latkml003.strokegen1_strokeLength, strokeGaps=latkml003.strokegen1_strokeGaps, shuffleOdds=latkml003.strokegen1_shuffleOdds, spreadPoints=latkml003.strokegen1_spreadPoints, limitPalette=latk.paletteLimit)
        return {'FINISHED'}


class latkml003_Button_StrokeGen2(bpy.types.Operator):
    """Generate GP strokes from a mesh"""
    bl_idname = "latkml003_button.strokegen2"
    bl_label = "StrokeGen 2"
    bl_options = {'UNDO'}
    
    def execute(self, context):
        latkml003 = context.scene.latkml003_settings
        strokeGen2(radius=latkml003.strokegen2_radius, minPointsCount=latkml003.strokegen2_minPointsCount)
        return {'FINISHED'}


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
        lb.setThickness(latkml003.latkml003_thickness)
        return {'FINISHED'}


class latkml003_Button_SingleFrame(bpy.types.Operator):
    """Operate on a single frame"""
    bl_idname = "latkml003_button.singleframe"
    bl_label = "Single Frame"
    bl_options = {'UNDO'}
    
    def execute(self, context):
        latkml003 = context.scene.latkml003_settings
        net1, net2 = loadModel()

        la = latk.Latk()
        la.layers.append(latk.LatkLayer())
        laFrame = doInference(net1, net2)
        la.layers[0].frames.append(laFrame)
        
        lb.fromLatkToGp(la, resizeTimeline=False)
        lb.setThickness(latkml003.latkml003_thickness)
        return {'FINISHED'}


# https://blender.stackexchange.com/questions/167862/how-to-create-a-button-on-the-n-panel
class latkml003Properties_Panel(bpy.types.Panel):
    """Creates a Panel in the 3D View context"""
    bl_idname = "GREASE_PENCIL_PT_latkml003PropertiesPanel"
    bl_space_type = 'VIEW_3D'
    bl_label = "latk-ml-003"
    bl_category = "Latk"
    bl_region_type = 'UI'
    #bl_context = "objectmode" # "mesh_edit"

    #def draw_header(self, context):
        #self.layout.prop(context.scene.freestyle_gpencil_export, "enable_latk", text="")

    def draw(self, context):
        layout = self.layout

        scene = context.scene
        latkml003 = scene.latkml003_settings

        row = layout.row()
        row.operator("latkml003_button.singleframe")
        row.operator("latkml003_button.allframes")

        row = layout.row()
        row.prop(latkml003, "latkml003_Model")

        row = layout.row()
        row.prop(latkml003, "latkml003_thickness")

        row = layout.row()
        row.operator("latkml003_button.strokegen1")
        row.prop(latkml003, "strokegen1_strokeLength")
        row.prop(latkml003, "strokegen1_strokeGaps")
        row.prop(latkml003, "strokegen1_shuffleOdds")
        row.prop(latkml003, "strokegen1_spreadPoints")

        row = layout.row()
        row.operator("latkml003_button.strokegen2")
        row.prop(latkml003, "strokegen2_radius")
        row.prop(latkml003, "strokegen2_minPointsCount")


classes = (
    OBJECT_OT_latkml003_prefs,
    latkml003Preferences,
    latkml003Properties,
    latkml003Properties_Panel,
	latkml003_Button_AllFrames,
	latkml003_Button_SingleFrame,
    latkml003_Button_StrokeGen1,
    latkml003_Button_StrokeGen2
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
    returns = modelSelector(latkml003.latkml003_Model.lower())
    return returns

def modelSelector(modelName):
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
    else:
        device = torch.device("cpu")
        return device

def createPyTorchNetwork(modelPath, net_G, input_nc=3, output_nc=1, n_blocks=3):
    device = getPyTorchDevice()
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


def strokeGen1(obj=None, strokeLength=1, strokeGaps=10.0, shuffleOdds=1.0, spreadPoints=0.1, limitPalette=32):
    if not obj:
        obj = lb.ss()
    mesh = obj.data
    mat = obj.matrix_world
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
    allPoints, allColors = lb.getVerts(target=obj, useWorldSpace=True, useColors=True, useBmesh=False)
    #~
    pointSeqsToAdd = []
    colorsToAdd = []
    for i in range(0, len(allPoints), strokeLength):
        color = None
        if not images:
            try:
                color = allColors[i]
            except:
                color = lb.getColorExplorer(obj, i)
        else:
            try:
                color = lb.getColorExplorer(obj, i, images)
            except:
                color = lb.getColorExplorer(obj, i)
        colorsToAdd.append(color)
        #~
        pointSeq = []
        for j in range(0, strokeLength):
            #point = allPoints[i]
            try:
                point = allPoints[i+j]
                if (len(pointSeq) == 0 or lb.getDistance(pointSeq[len(pointSeq)-1], point) < strokeGaps):
                    pointSeq.append(point)
            except:
                break
        if (len(pointSeq) > 0): 
            pointSeqsToAdd.append(pointSeq)
    for i, pointSeq in enumerate(pointSeqsToAdd):
        color = colorsToAdd[i]
        #createColor(color)
        if (limitPalette == 0):
            lb.createColor(color)
        else:
            lb.createAndMatchColorPalette(color, limitPalette, 5) # num places
        #stroke = frame.strokes.new(getActiveColor().name)
        #stroke.draw_mode = "3DSPACE"
        stroke = frame.strokes.new()
        stroke.display_mode = '3DSPACE'
        stroke.line_width = 10 # adjusted from 100 for 2.93
        stroke.material_index = gp.active_material_index

        stroke.points.add(len(pointSeq))

        if (random.random() < shuffleOdds):
            random.shuffle(pointSeq)

        for j, point in enumerate(pointSeq):    
            x = point[0] + (random.random() * 2.0 * spreadPoints) - spreadPoints
            y = point[2] + (random.random() * 2.0 * spreadPoints) - spreadPoints
            z = point[1] + (random.random() * 2.0 * spreadPoints) - spreadPoints
            pressure = 1.0
            strength = 1.0
            lb.createPoint(stroke, j, (x, y, z), pressure, strength)

def strokeGen2(obj=None, radius=2, minPointsCount=10, limitPalette=32):
    if not obj:
        obj = lb.ss()
    mesh = obj.data
    mat = obj.matrix_world
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
    allPoints, allColors = lb.getVerts(target=obj, useWorldSpace=True, useColors=True, useBmesh=False)
    #~
    strokeGroups = lb.group_points_into_strokes(allPoints, radius, minPointsCount)

    for i, strokeGroup in enumerate(strokeGroups):
        colorIndex = strokeGroup[0]

        if not images:
            try:
                color = allColors[colorIndex]
            except:
                color = lb.getColorExplorer(obj, colorIndex)
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
            x = point[0]
            y = point[2]
            z = point[1]
            pressure = 1.0
            strength = 1.0
            lb.createPoint(stroke, j, (x, y, z), pressure, strength)