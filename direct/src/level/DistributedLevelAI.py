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

    def __init__(self, air, zoneId, entranceId, avIds):
        DistributedObjectAI.DistributedObjectAI.__init__(self, air)
        Level.Level.__init__(self)
        # these are required fields
        self.zoneId = zoneId
        self.entranceId = entranceId

        assert len(avIds) > 0 and len(avIds) <= 4
        assert 0 not in avIds
        assert None not in avIds
        self.avIdList = avIds
        self.numPlayers = len(self.avIdList)
        self.notify.debug("expecting avatars: %s" % str(self.avIdList))

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

    def getPlayerIds(self):
        return self.avIdList

    def getEntranceId(self):
        return self.entranceId

    def delete(self):
        self.notify.debug('delete')
        if __debug__:
            self.removeAutosaveTask()
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
            self.accept(self.editMgrEntity.getSpecSaveEvent(), self.saveSpec)

    def createEntityCreator(self):
        """Create the object that will be used to create Entities.
        Inheritors, override if desired."""
        return EntityCreatorAI.EntityCreatorAI(level=self)

    def setOuch(self, penalty):
        avId = self.air.msgSender
        av = self.air.doId2do.get(avId)
        self.notify.debug("setOuch %s" % penalty)
        # make sure penalty is > 0
        if av and (penalty > 0):
            curHp = av.getHp()
            newHp = max(0, curHp-penalty)
            av.b_setHp(newHp)
        
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
            self.autosaveTaskName = self.uniqueName('autosaveSpec')
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
            # don't need to hit disk if we're just sending 'None' over the wire
            useDisk = 0
            if hash(self.levelSpec) != specHash:
                spec = self.levelSpec
                useDisk=simbase.config.GetBool('spec-by-disk', 0)
            specStr = repr(spec)

            import DistributedLargeBlobSenderAI
            largeBlob = DistributedLargeBlobSenderAI.\
                        DistributedLargeBlobSenderAI(
                self.air, self.zoneId, senderId, specStr,
                useDisk=useDisk)
            self.sendUpdateToAvatarId(senderId,
                                      'setSpecSenderDoId', [largeBlob.doId])
