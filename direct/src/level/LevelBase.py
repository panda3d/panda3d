"""LevelBase.py: contains the LevelBase class"""

import DirectNotifyGlobal
import string
from PythonUtil import lineInfo

"""
Any data that can be edited by a level editor must be represented as
an attribute of an entity owned by the level, in order to keep the
level-editing interface simple and unchanging.

To support this, we have entities such as 'levelMgr' and 'zoneEntity' that
contain crucial level information, much of which is needed when setting
up the level object, and is needed before other entity types can be
effectively created. (If you try to create a distributed entity, but
you don't yet have the information for the zone that it's in, because
you haven't created the zone's ZoneEntity, you're hurting.)


"""

"""
ZONE TERMINOLOGY
zoneNum / modelZoneNum : the number that a modeler chooses for a zone
zoneEntId : the entity ID of the ZoneEntity that represents a zone
zoneId : the network ID of the zone
"""

class LevelBase:
    """LevelBase: shared client and AI code
    representation of a game level, keeps track of all of the
    Level Entities and their interrelations"""
    notify = DirectNotifyGlobal.directNotify.newCategory('LevelBase')

    UberZoneEntId = 0

    def __init__(self):
        pass

    def initializeLevel(self, levelId, spec, scenarioIndex):
        """ subclass should call this as soon as it has located
        its spec data """
        self.levelId = levelId
        self.spec = spec
        self.scenarioIndex = scenarioIndex

        # create a complete set of global and scenario-specific entity specs
        globalEntities = self.spec['globalEntities']
        scenarioEntities = self.spec['scenarios'][self.scenarioIndex][0]
        entId2spec = {}
        entId2spec.update(globalEntities)
        entId2spec.update(scenarioEntities)
        self.entId2spec = entId2spec

        # create some handy tables

        # entity type -> list of entIds
        self.entType2ids = {}
        for entId, spec in self.entId2spec.items():
            entType = spec['type']
            self.entType2ids.setdefault(entType, [])
            self.entType2ids[entType].append(entId)

        # there should be one and only one levelMgr
        assert len(self.entType2ids['levelMgr']) == 1

        # get an entity creator object
        self.entityCreator = self.createEntityCreator()
        # set up handlers for entity creation
        self.setupEntityCreationHandlers()
        # create all the entities
        self.createAllEntities(priorityTypes=['levelMgr','zone'])
        # tear down the entity creation handlers
        self.removeEntityCreationHandlers()

    def destroyLevel(self):
        if hasattr(self, 'createdEntities'):
            # destroy the entities in reverse order
            while len(self.createdEntities) > 0:
                entity = self.createdEntities.pop()
                self.notify.debug('destroying %s entity %s' %
                                  (self.getEntityType(entity.entId),
                                   entity.entId))
                entity.destroy()
            del self.createdEntities
        del self.entities
        del self.entId2spec
        del self.spec

    def createEntityCreator(self):
        self.notify.error(
            'concrete Level class must override %s' % lineInfo()[2])

    def setupEntityCreationHandlers(self):
        # set up any handlers for entity creation events
        # override if desired, but be sure to call down

        # NOTE: we currently don't support a single object listening
        # multiple times to a single event; therefore, anything that
        # is listened for here cannot be listened for in a subclass
        # as well; one will stomp the other. Therefore, until we figure
        # out a better way to handle it, override any handlers defined
        # in LevelBase and call down, instead of installing your own
        # handler.

        self.acceptOnce(
            self.getEntityTypeCreateEvent('zone'),
            self.handleAllZonesCreated)

    def removeEntityCreationHandlers(self):
        pass

    def handleAllZonesCreated(self):
        """once all the zone entities have been created, and we've got a
        list of zoneIds in self.zoneIds, init zone tables"""
        # create a table mapping the model's zone numbers to the zone
        # entIds; zone entities are tied to model zone nums in the level spec,
        # this is just for convenient lookup
        self.zoneNum2entId = {}
        for entId in self.entType2ids['zone']:
            zoneEnt = self.getEntity(entId)
            self.zoneNum2entId[zoneEnt.modelZoneNum] = entId

        # At this point, we need to have a 'self.zoneIds' table of network
        # zoneIds, one for each zone including the UberZone. This is where
        # we decide which zoneNum/Entity gets mapped to which zoneId.
        # We sort the zoneNums, and then pair the sorted zoneNums up with the
        # zoneIds in the order that they appear in the self.zoneIds table.
        modelZoneNums = self.zoneNum2entId.keys()
        modelZoneNums.sort()
        # maps of zoneNum and zoneEntId to network zoneId
        self.zoneNum2zoneId = {}
        self.zoneEntId2zoneId = {}
        for i in range(len(modelZoneNums)):
            modelZoneNum = modelZoneNums[i]
            zoneEntId = self.zoneNum2entId[modelZoneNum]
            zoneId = self.zoneIds[i]
            self.zoneNum2zoneId[modelZoneNum] = zoneId
            self.zoneEntId2zoneId[zoneEntId] = zoneId

    def createAllEntities(self, priorityTypes=[]):
        """creates all entities in the spec. priorityTypes is an
        optional ordered list of entity types to create first."""
        # this will be filled in as the entities are created and report in
        # this includes distributed objects on the client
        self.entities = {}
        # this list contains the entities that we have actually created
        self.createdEntities = []

        # get list of all entity types we need to create
        entTypes = self.entType2ids.keys()

        # first create the types in the priority list
        for type in priorityTypes:
            assert type in entTypes
            self.createAllEntitiesOfType(type)
            entTypes.remove(type)

        # create the other entities in any old order
        for type in entTypes:
            self.createAllEntitiesOfType(type)

    def createAllEntitiesOfType(self, entType):
        """creates all entities of a given type"""
        assert entType in self.entType2ids
        for entId in self.entType2ids[entType]:
            self.createEntity(entId)
        # send the entity type-create event
        messenger.send(self.getEntityTypeCreateEvent(entType))

    def createEntity(self, entId):
        assert not self.entities.has_key(entId)
        spec = self.entId2spec[entId]
        self.notify.debug('creating %s %s' % (spec['type'], entId))
        entity = self.entityCreator.createEntity(entId)
        if entity is not None:
            self.createdEntities.append(entity)
        return entity

    def initializeEntity(self, entity):
        """populate an entity with its spec data. This is not done
        in createEntity in order to allow other pieces of code create
        entities; this is called directly by Entity.
        """
        entId = entity.entId
        spec = self.entId2spec[entId]
        # on initialization, set items directly on entity
        for key,value in spec.items():
            if key in ('type', 'name', 'comment',):
                continue
            entity.setAttribInit(key, value)

        # entity is initialized, add it to the list of entities
        self.entities[entity.entId] = entity
        # send the entity-create event
        messenger.send(self.getEntityCreateEvent(entity.entId))
        # send the entity-of-type create event
        messenger.send(self.getEntityOfTypeCreateEvent(spec['type']),
                                                       [entId])

    def getEntityTypeCreateEvent(self, entType):
        """This is the event that is thrown immediately after every
        entity of a given type has been created"""
        return 'entityTypeCreate-%s-%s' % (self.levelId, entType)

    def getEntityOfTypeCreateEvent(self, entType):
        """This event is thrown immediately after each instance of the
        given entity type is created; handlers must accept an entId"""
        return 'entityOfTypeCreate-%s-%s' % (self.levelId, entType)

    def getEntityCreateEvent(self, entId):
        """This is the event that is thrown immediately after an entity
        is initialized"""
        return 'entityCreate-%s-%s' % (self.levelId, entId)

    def getEntity(self, entId):
        return self.entities[entId]

    def getEntityType(self, entId):
        return self.entId2spec[entId]['type']

    def getZoneId(self, dummy=None, zoneNum=None, entId=None):
        """look up network zoneId by zoneNum or entId"""
        assert (zoneNum is not None) or (entId is not None)
        assert not ((zoneNum is not None) and (entId is not None))
        if zoneNum is not None:
            assert zoneNum in self.zoneNum2zoneId
            return self.zoneNum2zoneId[zoneNum]
        else:
            assert entId in self.zoneEntId2zoneId
            return self.zoneEntId2zoneId[entId]
