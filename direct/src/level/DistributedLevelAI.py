"""DistributedLevelAI.py: contains the DistributedLevelAI class"""

from ClockDelta import *
import DistributedObjectAI
import LevelBase
import DirectNotifyGlobal
import EntityCreatorAI
import WeightedChoice

class DistributedLevelAI(DistributedObjectAI.DistributedObjectAI,
                         LevelBase.LevelBase):
    """DistributedLevelAI"""
    notify = DirectNotifyGlobal.directNotify.newCategory('DistributedLevelAI')

    def __init__(self, air):
        DistributedObjectAI.DistributedObjectAI.__init__(self, air)
        LevelBase.LevelBase.__init__(self)

    def initializeLevel(self, spec, uberZoneId):
        self.uberZoneId = uberZoneId

        # choose a scenario
        wc = WeightedChoice.WeightedChoice(spec['scenarios'], 1)
        scenario = wc.choose()
        scenarioIndex = spec['scenarios'].index(scenario)

        LevelBase.LevelBase.initializeLevel(self, spec, scenarioIndex)

        # allocate the rest of the zones; add one for the uber-zone
        self.numZones = len(self.spec['zones']) + 1
        self.zoneIds = [self.uberZoneId]
        for i in range(1,self.numZones):
            # there is error checking in air.allocateZone
            self.zoneIds.append(self.air.allocateZone())

        # record the level's start time so that we can sync the clients
        self.startTime = globalClock.getRealTime()
        self.startTimestamp = globalClockDelta.localToNetworkTime(
            self.startTime, bits=32)

    # required-field getters
    def getZoneIds(self):
        return self.zoneIds

    def getStartTimestamp(self):
        return self.startTimestamp

    def getScenarioIndex(self):
        return self.scenarioIndex

    def generate(self):
        self.notify.debug('generate')
        DistributedObjectAI.DistributedObjectAI.generate(self)

        self.setLevelId(self.doId)

        # create the Entities
        self.aiEntities = {}
        for entId, spec in self.entId2Spec.iteritems():
            self.notify.debug('creating %s %s' % (spec['type'], entId))
            entity = EntityCreatorAI.createEntity(
                spec['type'], self.air, self.doId, entId,
                self.getZoneId(spec['zone']))
            if entity is not None:
                self.aiEntities[entId] = entity

    def delete(self):
        self.notify.debug('delete')
        
        for entId in self.aiEntities.keys():
            self.aiEntities[entId].destroy()
        del self.aiEntities
        
        # we do not allocate the uberZone for now, so don't deallocate it
        for zoneId in self.zoneIds[1:]:
            self.air.deallocateZone(zoneId)

        self.destroyLevel()

        DistributedObjectAI.DistributedObjectAI.delete(self)
