""" Class used to create and control radamec device """
from math import *
from PandaObject import *
from DirectDeviceManager import *

import DirectNotifyGlobal
import OnscreenText


"""
TODO:
Handle interaction between widget, followSelectedTask and updateTask
"""

# ANALOGS
NULL_AXIS = -1
RAD_PAN = 0
RAD_TILT = 1
RAD_ZOOM = 2
RAD_FOCUS = 3

class DirectRadamec(PandaObject):
    radamecCount = 0
    notify = DirectNotifyGlobal.directNotify.newCategory('DirectRadamec')
    
    def __init__(self, device = 'Analog0', nodePath = direct.camera):
        # See if device manager has been initialized
        if direct.deviceManager == None:
            direct.deviceManager = DirectDeviceManager()
        # Set name
        DirectRadamec.radamecCount += 1
        self.name = 'Radamec-' + `DirectRadamec.radamecCount`
        # Get analogs
        self.device = device
        self.analogs = direct.deviceManager.createAnalogs(self.device)
        self.aList = [0,0,0,0,0,0,0,0]
        # Radamec device max/mins - measured on 7/31/2001 - Samir
        # Note:  These values change quite often, i.e. everytime
        #        you unplug the radamec cords, or jostle them too
        #        much.  For best results, re-record these values often.
        self.minRange = [-180.0, -90, 524290.0, 520315.0]
        self.maxRange = [180.0, 90, 542700.0, 559518.0]
        # Pick initial mode
        self.updateFunc = self.radamecUpdate
        # Spawn update task
        self.enable()
        
    def enable(self):
        # Kill existing task
        self.disable()
        # Update task
        taskMgr.spawnMethodNamed(self.updateTask, self.name + '-updateTask')
    
    def disable(self):
        taskMgr.removeTasksNamed(self.name + '-updateTask')

    def destroy(self):
        self.disable()

    def updateTask(self, state):
        self.updateVals()
        self.updateFunc()
        return Task.cont
    
    def updateVals(self):
        numControls = self.analogs.__len__()
        self.notify.debug("We have %s analog controls." % numControls)
        # Update analogs
        for i in range(len(self.analogs)):
            self.aList[i] = self.analogs[i]

    def radamecUpdate(self):
        panVal = self.normalizeChannel(RAD_PAN,-180,180)
        tiltVal = self.normalizeChannel(RAD_TILT,-90,90)

        self.notify.debug("PAN = %s" % self.aList[RAD_PAN])
        self.notify.debug("TILT = %s" % self.aList[RAD_TILT])
        self.notify.debug("ZOOM = %s" % self.aList[RAD_ZOOM])
        self.notify.debug("FOCUS = %s" % self.aList[RAD_FOCUS])
        self.notify.debug("Normalized: panVal: %s  tiltVal: %s" % (panVal, tiltVal))

    # Normalize to the range [-minVal,maxVal] based on some hard-coded
    # max/min numbers of the Radamec device
    def normalizeChannel(self, chan, minVal = -1, maxVal = 1):
        try:
            maxRange = self.maxRange[chan]
            minRange = self.minRange[chan]
        except IndexError:
            raise RuntimeError, "can't normalize this channel (chanel %d)" % chan
        range = maxRange - minRange
        clampedVal = CLAMP(self.aList[chan], minRange, maxRange)
        return ((maxVal - minVal) * (clampedVal - minRange) / range) + minVal
