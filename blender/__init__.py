import sys
import pathlib
import bpy

root = pathlib.Path(__file__).parent
sys.path.insert(0, str(root))
sys.path.insert(0, str(root / 'lib' / 'windows-py310'))

if __name__ != "__main__":
    from . import connector

bl_info = {
    "name": "Mojo",
    "blender": (3, 3, 2),
    "category": "Animation",
}


def register():
    bpy.utils.register_class(connector.MojoConnector)
    bpy.types.TOPBAR_MT_edit.append(connector.menu_func)


def unregister():
    bpy.utils.unregister_class(connector.MojoConnector)
