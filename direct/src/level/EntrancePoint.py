from toontown.toonbase.ToontownGlobals import *
from direct.directnotify import DirectNotifyGlobal
import BasicEntities
from pandac import NodePath

class EntrancePoint(BasicEntities.NodePathEntity):
    def __init__(self, level, entId):
        BasicEntities.NodePathEntity.__init__(self, level, entId)
        self.rotator = self.attachNewNode('rotator')
        self.placer = self.rotator.attachNewNode('placer')
        self.initEntrancePoint()

    def destroy(self):
        self.destroyEntrancePoint()
        self.placer.removeNode()
        self.rotator.removeNode()
        del self.placer, self.rotator
        BasicEntities.NodePathEntity.destroy(self)

    def placeToon(self, toon, toonIndex, numToons):
        self.placer.setY(-self.radius)
        self.rotator.setH((-self.theta*(numToons-1)*.5) +
                          (toonIndex*self.theta))
        toon.setPosHpr(self.placer, 0,0,0, 0,0,0)

    def initEntrancePoint(self):
        if self.entranceId >= 0:
            self.level.entranceId2entity[self.entranceId] = self

    def destroyEntrancePoint(self):
        if self.entranceId >= 0:
            if self.level.entranceId2entity.has_key(self.entranceId):
                del self.level.entranceId2entity[self.entranceId]

    if __dev__:
        def attribChanged(self, *args):
            BasicEntities.NodePathEntity.attribChanged(self, *args)
            self.destroyEntrancePoint()
            self.initEntrancePoint()
