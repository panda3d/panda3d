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

        # We need a unique level ID to init the level system, and we need
        # the level system to get the data for our required fields.
        # Pre-allocate a doId.
        self.preAllocateDoId()

        # choose a scenario
        wc = WeightedChoice.WeightedChoice(spec['scenarios'], 1)
        scenario = wc.choose()
        scenarioIndex = spec['scenarios'].index(scenario)

        LevelBase.LevelBase.initializeLevel(self, self.doId,
                                            spec, scenarioIndex)
        self.aiEntities = {}
        # get list of entity types we need to create
        self.entTypes = self.entType2Ids.keys()

        # create the levelMgr
        self.levelMgr = self.createEntity(self.entType2Ids['levelMgr'][0])
        self.entTypes.remove('levelMgr')

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

    def makeEntityCreator(self):
        """Create the object that will be used to create Entities.
        Inheritors, override if desired."""
        return EntityCreatorAI.EntityCreatorAI()

    def createEntity(self, entId):
        assert not self.aiEntities.has_key(entId)
        spec = self.entId2Spec[entId]
        self.notify.debug('creating %s %s' % (spec['type'], entId))
        zone = spec.get('zone')
        # we might attempt to create non-distributed entities before
        # we've been generated
        if zone is not None:
            zone = self.getZoneId(zone)
            entity = self.entityCreator.createEntity(
                spec['type'], self, entId, self.air, zone)
        else:
            entity = self.entityCreator.createEntity(
                spec['type'], self, entId)
        if entity is not None:
            self.aiEntities[entId] = entity
        return entity

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

        # create the rest of the Entities
        for entType in self.entTypes:
            for entId in self.entType2Ids[entType]:
                self.createEntity(entId)

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
