import bpy
import MojoBlender
import pencil

connector = None


class BlenderConnector(MojoBlender.Connector):
    def __init__(self):
        super().__init__()

    def topic(self):
        return "blender"

    def on_frame_change(self, frame):
        bpy.context.scene.frame_current = frame

    def on_pencil_change(self, path, pencil):
        print("pencil change:", path)


def try_recv():
    connector.try_recv()
    return 0.01


def get_full_path(obj):
    segments = []
    while obj:
        segments.append(obj.name)
        obj = obj.parent
    return '/' + '/'.join(segments[::-1])


def on_depsgraph_update(scene):
    obj = bpy.context.active_object
    if obj and isinstance(obj.data, bpy.types.GreasePencil):
        path = '{}/{}'.format(get_full_path(obj), obj.data.name)
        data = pencil.export(obj.data)
        connector.send_pencil(path, data)
    return {'FINISHED'}


def connect():
    global connector
    if connector:
        return

    connector = BlenderConnector()
    bpy.app.timers.register(try_recv)
    bpy.app.handlers.frame_change_pre.append(
        lambda scene: connector.send_frame(scene.frame_current))
    bpy.app.handlers.depsgraph_update_post.append(on_depsgraph_update)


def disconnect():
    global connector
    bpy.app.timers.unregister(try_recv)
    bpy.app.handlers.frame_change_pre.clear()
    bpy.app.handlers.depsgraph_update_post.clear()
    if connector:
        del connector
        connector = None


class MojoConnector(bpy.types.Operator):
    bl_idname = "edit.connect_mojo"
    bl_label = "Connect to mojo"
    bl_options = {'REGISTER'}

    def execute(self, context):
        connect()
        return {'FINISHED'}


class MojoDisconnector(bpy.types.Operator):
    bl_idname = "edit.disconnect_mojo"
    bl_label = "Close mojo"
    bl_options = {'REGISTER'}

    def execute(self, context):
        disconnect()
        return {'FINISHED'}


def menu_func(self, context):
    self.layout.operator(MojoConnector.bl_idname)
    self.layout.operator(MojoDisconnector.bl_idname)
