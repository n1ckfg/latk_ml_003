bl_info = {
    "name": "latk-ml-003", 
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

class latkml003_Button_AllFrames(bpy.types.Operator):
    """Operate on all frames"""
    bl_idname = "latkml003_button.allframes"
    bl_label = "All Frames"
    bl_options = {'UNDO'}
    
    def execute(self, context):
        # function goes here
        pass
        return {'FINISHED'}

class latkml003_Button_SingleFrame(bpy.types.Operator):
    """Operate on a single frame"""
    bl_idname = "latkml003_button.singleframe"
    bl_label = "Single Frame"
    bl_options = {'UNDO'}
    
    def execute(self, context):
        # function goes here
        pass
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
        #row.prop(latkml003, "material_shader_mode")

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

