""" Class used to create and control radamec device """
from math import *
from direct.showbase.DirectObject import DirectObject
from DirectDeviceManager import *

from direct.directnotify import DirectNotifyGlobal

"""
TODO:
Handle interaction between widget, followSelectedTask and updateTask
"""

# ANALOGS
NULL_AXIS = -1
FAST_X = 0
FAST_Y = 1
FAST_Z = 2

class DirectFastrak(DirectObject):
    fastrakCount = 0
    notify = DirectNotifyGlobal.directNotify.newCategory('DirectFastrak')

    def __init__(self, device = 'Tracker0', nodePath = base.direct.camera):
        # See if device manager has been initialized
        if base.direct.deviceManager == None:
            base.direct.deviceManager = DirectDeviceManager()

        # Set name
        self.name = 'Fastrak-' + `DirectFastrak.fastrakCount`
        self.deviceNo = DirectFastrak.fastrakCount
        DirectFastrak.fastrakCount += 1

        # Get analogs
        self.device = device
        self.tracker = None
        self.trackerPos = None

        # Spawn update task
        self.updateFunc = self.fastrakUpdate
        self.enable()

    def enable(self):
        # Kill existing task
        self.disable()
        # Initialize tracker
        self.tracker = base.direct.deviceManager.createTracker(self.device)
        # Update task
        taskMgr.add(self.updateTask, self.name + '-updateTask')

    def disable(self):
        taskMgr.remove(self.name + '-updateTask')

    def destroy(self):
        self.disable()
        self.tempCS.removeNode()

    def updateTask(self, state):
        self.updateFunc()
        return Task.cont

    def fastrakUpdate(self):
        # Get tracker position in feet.  Flip x, z axes.
        pos = base.direct.fastrak[self.deviceNo].tracker.getPos()
        self.trackerPos = Vec3(3.280839895013123 * pos[2],
                               3.280839895013123 * pos[1],
                               3.280839895013123 * pos[0])
        self.notify.debug("Tracker(%d) Pos = %s" % (self.deviceNo, `self.trackerPos`))

