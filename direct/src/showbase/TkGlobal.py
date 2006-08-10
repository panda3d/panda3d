"""Undocumented Module"""

__all__ = ['taskMgr']

from Tkinter import *
from direct.task.TaskManagerGlobal import *
from direct.task.Task import Task
import Pmw
import sys

# This is required by the ihooks.py module used by Squeeze (used by
# pandaSqueezer.py) so that Pmw initializes properly
sys.modules['_Pmw'].__name__ = '_Pmw'

__builtins__["tkroot"] = Pmw.initialise()

def tkLoop(self):
    # Do all the tkinter events waiting on this frame
    # dooneevent will return 0 if there are no more events
    # waiting or 1 if there are still more.
    # DONT_WAIT tells tkinter not to block waiting for events
    while tkinter.dooneevent(tkinter.ALL_EVENTS | tkinter.DONT_WAIT):
        pass
    # Run forever
    return Task.cont

def spawnTkLoop():
    # Spawn this task
    taskMgr.add(tkLoop, "tkLoop")

taskMgr.remove('tkLoop')
spawnTkLoop()
