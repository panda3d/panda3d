"""LevelBase.py: contains the LevelBase class"""

import DirectNotifyGlobal
import string
from PythonUtil import lineInfo

class LevelBase:
    """LevelBase: shared client and AI code
    representation of a game level, keeps track of all of the
    Level Entities and their interrelations"""
    notify = DirectNotifyGlobal.directNotify.newCategory('LevelBase')

    def __init__(self, levelId=None):
        if levelId is not None:
            self.setLevelId(levelId)

    def setLevelId(self, levelId):
        self.levelId = levelId

    def initializeLevel(self, spec, scenarioIndex):
        """ subclass should call this as soon as it has located
        its spec data """
        self.spec = spec
        self.scenarioIndex = scenarioIndex

        # create a complete set of global and scenario-specific entity specs
        globalEntities = self.spec['globalEntities']
        scenarioEntities = self.spec['scenarios'][self.scenarioIndex][0]
        entId2Spec = {}
        entId2Spec.update(globalEntities)
        entId2Spec.update(scenarioEntities)
        self.entId2Spec = entId2Spec

        # this will be filled in as the entities are created and report in
        self.entities = {}

        # get an entity creator object
        self.entityCreator = self.makeEntityCreator()

    def makeEntityCreator(self):
        self.notify.error(
            'concrete Level class must override %s' % lineInfo()[2])

    def destroyLevel(self):
        del self.entities
        del self.entId2Spec
        del self.spec

    def initializeEntity(self, entity):
        """populate an entity with its spec data"""
        entId = entity.entId
        spec = self.entId2Spec[entId]
        # on initialization, set items directly on entity
        for key,value in spec.items():
            if key in ('type', 'name', 'comment',):
                continue
            if hasattr(entity, key):
                self.notify.warning('entity %s (%s) already has member %s' %
                                    (entId, spec['type'], key))
            entity.__dict__[key] = value

        # entity is initialized, add it to the list of entities
        self.entities[entity.entId] = entity
        # send the create event
        messenger.send(self.getEntityCreateEvent(entity.entId))

        """
        # set items directly on entity, or call callback functions...
        for key,value in spec.items():
            # filter out some entries; we might want to restructure the
            # spec data into different categories of properties instead
            # of filtering them here
            if key in ('type', 'name', 'comment', 'zone',):
                continue
            setFuncName = 'set%s%s' % (string.upper(key[0]), key[1:])
            if hasattr(entity, setFuncName):
                # call the setter
                func = getattr(entity, setFuncName)
                func(value)
            else:
                # set the param directly on the object
                entity.__dict__[key] = value
                """

    def getEntityCreateEvent(self, entId):
        """This is the event that is thrown immediately after an entity
        is initialized"""
        return 'entityCreate-%s-%s' % (self.levelId, entId)

    def getEntity(self, entId):
        return self.entities[entId]

    def getEntityType(self, entId):
        return self.entId2Spec[entId]['type']

    def getZoneId(self, index):
        # get the actual zoneId for this index
        # TODO: perhaps the zones should be fixed up in the specs
        return self.zoneIds[index]
