import wx
from direct.task.Task import Task

def wxLoop(self):
    # First we need to ensure that the OS message queue is processed.
    base.wxApp.Yield()

    # Now do all the wxPython events waiting on this frame.
    while base.wxApp.Pending():
        base.wxApp.Dispatch()

    return Task.cont

def spawnWxLoop():
    if not getattr(base, 'wxApp', None):
        # Create a new base.wxApp, but only if it's not already
        # created.
        base.wxApp = wx.PySimpleApp(redirect = False)

    # Spawn this task
    taskMgr.remove('wxLoop')
    taskMgr.add(wxLoop, "wxLoop")
