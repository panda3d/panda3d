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

    def __init__(self, air, zoneId):
        DistributedObjectAI.DistributedObjectAI.__init__(self, air)
        LevelBase.LevelBase.__init__(self)
        self.uberZoneId = zoneId

    def generate(self, spec):
        self.notify.debug('generate')
        DistributedObjectAI.DistributedObjectAI.generate(self)

        self.initializeLevel(spec)

        self.sendUpdate('setZoneIds', [self.zoneIds])
        self.sendUpdate('setStartTimestamp', [self.startTimestamp])
        self.sendUpdate('setScenarioIndex', [self.scenarioIndex])

    def delete(self):
        self.notify.debug('delete')
        
        # we do not allocate the uberZone for now, so don't deallocate it
        for zoneId in self.zoneIds[1:]:
            self.air.deallocateZone(zoneId)

        self.destroyLevel()

        DistributedObjectAI.DistributedObjectAI.delete(self)

    def initializeLevel(self, spec):
        # record the level's start time so that we can sync the clients
        self.startTime = globalClock.getRealTime()
        self.startTimestamp = globalClockDelta.localToNetworkTime(
            self.startTime, bits=32)

        # choose a scenario
        wc = WeightedChoice.WeightedChoice(spec['scenarios'], 1)
        scenario = wc.choose()
        scenarioIndex = spec['scenarios'].index(scenario)

        # this will hold the network zoneIds that we allocate
        self.zoneIds = [self.uberZoneId]

        LevelBase.LevelBase.initializeLevel(self, self.doId,
                                            spec, scenarioIndex)

    def createEntityCreator(self):
        """Create the object that will be used to create Entities.
        Inheritors, override if desired."""
        return EntityCreatorAI.EntityCreatorAI(self.air, level=self)

    def setupEntityCreationHandlers(self):
        LevelBase.LevelBase.setupEntityCreationHandlers(self)
        # listen for the creation of each zone object
        self.accept(self.getEntityOfTypeCreateEvent('zone'),
                    self.zoneEntCreated)

    def removeEntityCreationHandlers(self):
        LevelBase.LevelBase.removeEntityCreationHandlers(self)
        self.ignore(self.getEntityOfTypeCreateEvent('zone'))

    def zoneEntCreated(self, entId):
        if entId == LevelBase.LevelBase.UberZoneEntId:
            return
        # there is error checking in air.allocateZone
        self.zoneIds.append(self.air.allocateZone())

    def getEntityZoneId(self, entId):
        """figure out what network zoneId an entity is in"""
        # TODO: where should the zone info come from? It could come
        # from the 'parent' scene-graph info... but what about intangible
        # distributed objects? I guess if they don't inherit from
        # NodePathEntity et al, they'll just have an unused 'parent'
        # attribute hanging around, which isn't the end of the world...

        # this func is called before the entity has been created; look
        # into the spec data, since we can't get a handle on the object itself
        spec = self.entId2spec[entId]
        type = spec['type']
        if type == 'zone':
            if not hasattr(self, 'zoneNum2zoneId'):
                # we haven't even created our zone entities yet;
                # we have no idea yet which zoneNums map to which
                # network zoneIds. just return None.
                return None
            return self.zoneNum2zoneId[spec['modelZoneNum']]
        if not spec.has_key('parent'):
            return None
        return self.getEntityZoneId(spec['parent'])

    if __debug__:
        # level editors should call this func to tweak attributes of level
        # entities
        def setAttribChange(self, entId, attribName, value):
            # send a copy to the client-side level obj
            self.sendUpdate('setAttribChange',
                            [entId, attribName, repr(value)])
            
            entity = self.getEntity(entId)
            entity.handleAttribChange(attribName, value)
