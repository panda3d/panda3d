
from Tkinter import *
import Pmw

import Task

def tkloop(self):
    # Do all the tkinter events waiting on this frame
    # dooneevent will return 0 if there are no more events
    # waiting or 1 if there are still more.
    # DONT_WAIT tells tkinter not to block waiting for events
    while tkinter.dooneevent(tkinter.DONT_WAIT):
        pass
    # Run forever
    return Task.cont

# Get the taskMgr
from TaskManagerGlobal import *

# Spawn this task
taskMgr.spawnTaskNamed(Task.Task(tkloop), "tkloop")
