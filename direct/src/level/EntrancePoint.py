from ToontownGlobals import *
import DirectNotifyGlobal
import BasicEntities
import NodePath

class EntrancePoint(BasicEntities.NodePathEntity):
    def __init__(self, level, entId):
        BasicEntities.NodePathEntity.__init__(self, level, entId)
        self.initEntrancePoint()

    def destroy(self):
        self.destroyEntrancePoint()
        BasicEntities.NodePathEntity.destroy(self)

    def placeToon(self, toon, toonIndex, numToons):
        toon.setPosHpr(self, 0,0,0, 0,0,0)

    def initEntrancePoint(self):
        if self.entranceId >= 0:
            self.level.entranceId2entity[self.entranceId] = self

    def destroyEntrancePoint(self):
        if self.entranceId >= 0:
            if self.level.entranceId2entity.has_key(self.entranceId):
                del self.level.entranceId2entity[self.entranceId]

    if __debug__:
        def attribChanged(self, *args):
            BasicEntities.NodePathEntity.attribChanged(self, *args)
            self.destroyEntrancePoint()
            self.initEntrancePoint()
