from __future__ import print_function
from maya import cmds
from mojo import connector


def on_load():
    cmds.menu('mojoMenu', label='Mojo', parent='MayaWindow')
    cmds.menuItem('mojoLanuch', label='Launch',
                  command=lambda _: connect(), parent='mojoMenu')
    cmds.menuItem('mojoClose', label='Close', enable=False,
                  command=lambda _: disconnect(), parent='mojoMenu')
    cmds.menuItem('mojoSync', label='Sync', enable=False,
                  command=lambda _: connector.send(cmds.ls(dag=True)), parent='mojoMenu')


def on_unload():
    cmds.deleteUI("mojoMenu", menu=True)


def connect():
    if connector.launch():
        cmds.menuItem('mojoLanuch', e=True, enable=False)
        cmds.menuItem('mojoClose', e=True, enable=True)
        cmds.menuItem('mojoSync', e=True, enable=True)


def disconnect():
    if connector.stop():
        cmds.menuItem('mojoLanuch', e=True, enable=True)
        cmds.menuItem('mojoClose', e=True, enable=False)
        cmds.menuItem('mojoSync', e=True, enable=False)
