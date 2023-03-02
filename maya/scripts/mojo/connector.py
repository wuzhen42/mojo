import MojoMaya
import subprocess
from maya import cmds

connector = None
jobs = []
server = None


class MayaConnector(MojoMaya.MayaConnector):
    def __init__(self):
        super(MayaConnector, self).__init__()

    def topic(self):
        return "maya"

    def on_frame_change(self, frame):
        cmds.currentTime(frame)

    def log(self, text):
        print(text)


def on_frame_change():
    frame = int(cmds.currentTime(q=True))
    connector.send_frame(frame)


def on_node_delete():
    pass


def launch(hide=False):
    global connector, jobs, server

    if connector:
        return False

    kwargs = dict()
    if hide:
        startupinfo = subprocess.STARTUPINFO()
        startupinfo.dwFlags = subprocess.CREATE_NEW_CONSOLE | subprocess.STARTF_USESHOWWINDOW
        startupinfo.wShowWindow = subprocess.SW_HIDE
        kwargs['startupinfo'] = startupinfo
    server = subprocess.Popen(['mojo'], **kwargs)
    connector = MayaConnector()

    jobs.append(cmds.scriptJob(e=["quitApplication", close_server]))
    jobs.append(cmds.scriptJob(idleEvent=connector.try_recv))
    # jobs.append(cmds.scriptJob(nodeDeleted=on_node_delete))
    jobs.append(cmds.scriptJob(e=["timeChanged", on_frame_change]))

    return True


def stop():
    global connector, jobs

    for id in jobs:
        cmds.scriptJob(kill=id, f=True)
    jobs = []

    if connector:
        del connector
        connector = None

    close_server()

    return True


def close_server():
    global server
    if server:
        server.kill()
        server = None


def send(objects):
    for obj in objects:
        if cmds.nodeType(obj) not in {'mojoPencil', 'camera'}:
            continue
        connector.send_maya(obj)
        # add handler on obj change, deleted or renamed
