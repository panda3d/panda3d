"""SampleObject module: contains the SampleObject class"""

from direct.directnotify.DirectNotifyGlobal import *
from direct.distributed.DistributedObject import *

class SampleObject(DistributedObject):

    notify = directNotify.newCategory("SampleObject")

    def __init__(self, cr):
        self.cr = cr
#        self.red = 0
#        self.green = 0
#        self.blue = 0

    def setColor(self, red = 0, green = 0, blue = 0):
        self.red = red
        self.green = green
        self.blue = blue
        self.announceGenerate()

    def getColor(self):
        return (self.red, self.green, self.blue)
