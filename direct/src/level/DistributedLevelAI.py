"""DistributedLevelAI.py: contains the DistributedLevelAI class"""

from AIBaseGlobal import *
from ClockDelta import *
import DistributedObjectAI
import Level
import DirectNotifyGlobal
import EntityCreatorAI
import WeightedChoice

class DistributedLevelAI(DistributedObjectAI.DistributedObjectAI,
                         Level.Level):
    """DistributedLevelAI"""
    notify = DirectNotifyGlobal.directNotify.newCategory('DistributedLevelAI')

    def __init__(self, air, zoneId):
        DistributedObjectAI.DistributedObjectAI.__init__(self, air)
        Level.Level.__init__(self)
        # this is one of the required fields
        self.zoneId = zoneId
        if __debug__:
            self.modified = 0

    def generate(self, levelSpec):
        self.notify.debug('generate')
        DistributedObjectAI.DistributedObjectAI.generate(self)

        self.initializeLevel(levelSpec)

        self.sendUpdate('setZoneIds', [self.zoneIds])
        self.sendUpdate('setStartTimestamp', [self.startTimestamp])
        self.sendUpdate('setScenarioIndex', [self.scenarioIndex])

    def getLevelZoneId(self):
        """no entities should be generated in the level's zone; it causes
        nasty race conditions on the client if there are entities in the
        same zone with the level"""
        return self.zoneId

    def delete(self):
        self.notify.debug('delete')
        if __debug__:
            self.removeBackupTask()
        self.destroyLevel()
        DistributedObjectAI.DistributedObjectAI.delete(self)

    def initializeLevel(self, levelSpec):
        # record the level's start time so that we can sync the clients
        self.startTime = globalClock.getRealTime()
        self.startTimestamp = globalClockDelta.localToNetworkTime(
            self.startTime, bits=32)

        # choose a scenario
        # make list of lists: [(weight, scenarioIndex), ...]
        lol = zip(levelSpec.getScenarioWeights(),
                  range(levelSpec.getNumScenarios()))
        wc = WeightedChoice.WeightedChoice(lol)
        scenarioIndex = wc.choose()[1]

        Level.Level.initializeLevel(self, self.doId, levelSpec, scenarioIndex)

        if __debug__:
            # listen for requests to save the spec
            editMgrEntId = self.entType2ids['editMgr'][0]
            editMgr = self.getEntity(editMgrEntId)
            self.accept(editMgr.getSpecSaveEvent(), self.saveSpec)

    def createEntityCreator(self):
        """Create the object that will be used to create Entities.
        Inheritors, override if desired."""
        return EntityCreatorAI.EntityCreatorAI(level=self)

    def getEntityZoneId(self, entId):
        """figure out what network zoneId an entity is in"""
        # this func is called before the entity has been created; look
        # into the spec data, since we can't yet get a handle on the
        # object itself at this point
        spec = self.levelSpec.getEntitySpec(entId)
        type = spec['type']
        if type == 'zone':
            if not hasattr(self, 'zoneNum2zoneId'):
                # we haven't even started creating our zone entities yet;
                # we have no idea yet which zoneNums map to which
                # network zoneIds. just return None.
                return None
            # If the entity *is* the zone, it will not yet be in the
            # table; but since zone entities are currently not distributed,
            # it's fine to return None.
            return self.zoneNum2zoneId.get(spec['modelZoneNum'])
        if not spec.has_key('parent'):
            return None
        return self.getEntityZoneId(spec['parent'])

    if __debug__:
        # level editors should call this func to tweak attributes of level
        # entities
        def setAttribChange(self, entId, attribName, value, username='SYSTEM'):
            # send a copy to the client-side level obj FIRST
            # (it may be a message that creates an entity)
            self.sendUpdate('setAttribChange',
                            [entId, attribName, repr(value), username])
            self.levelSpec.setAttribChange(entId, attribName, value, username)

            self.modified = 1
            self.scheduleAutosave()

        # backups are made every N minutes, starting from the time that
        # the first edit is made
        AutosavePeriod = simbase.config.GetFloat(
            'level-autosave-period-minutes', 5)

        def scheduleAutosave(self):
            if hasattr(self, 'autosaveTask'):
                return
            self.autosaveTaskName = self.uniqueName('saveSpec')
            self.autosaveTask = taskMgr.doMethodLater(
                DistributedLevelAI.AutosavePeriod * 60,
                self.autosaveSpec,
                self.autosaveTaskName)

        def removeAutosaveTask(self):
            if hasattr(self, 'autosaveTask'):
                taskMgr.remove(self.autosaveTaskName)
                del self.autosaveTask

        def autosaveSpec(self, task=None):
            self.removeAutosaveTask()
            if self.modified:
                DistributedLevelAI.notify.info('autosaving spec')
                filename = self.levelSpec.getFilename()
                filename = '%s.autosave' % filename
                self.levelSpec.saveToDisk(filename, makeBackup=0)

        def saveSpec(self, task=None):
            DistributedLevelAI.notify.info('saving spec')
            self.removeAutosaveTask()
            if not self.modified:
                DistributedLevelAI.notify.info('no changes to save')
                return
            self.levelSpec.saveToDisk()
            self.modified = 0

        def requestCurrentLevelSpec(self, specHash, entTypeRegHash):
            senderId = self.air.msgSender

            # first check the typeReg hash -- if it doesn't match, the
            # client should not be connecting. Their entityTypeRegistry
            # is different from ours.
            srvHash = hash(self.levelSpec.entTypeReg)
            if srvHash != entTypeRegHash:
                self.sendUpdateToAvatarId(
                    senderId, 'setSpecDeny',
                    ['EntityTypeRegistry hashes do not match! '
                     '(server:%s, client:%s' % (srvHash, entTypeRegHash)])
                return
            
            spec = None
            if hash(self.levelSpec) != specHash:
                spec = self.levelSpec
            specStr = repr(spec)

            import DistributedLargeBlobSenderAI
            largeBlob = DistributedLargeBlobSenderAI.\
                        DistributedLargeBlobSenderAI(
                self.air, self.zoneId, senderId, specStr,
                useDisk=simbase.config.GetBool('spec-by-disk', 0))
            self.sendUpdateToAvatarId(senderId,
                                      'setSpecSenderDoId', [largeBlob.doId])
