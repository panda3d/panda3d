"""AIRepository module: contains the AIRepository class"""

from direct.distributed.ConnectionRepository import ConnectionRepository
from panda3d.core import *
from direct.directnotify import DirectNotifyGlobal
from direct.distributed.MsgTypes import *
from direct.distributed.PyDatagram import PyDatagram
from direct.distributed.PyDatagramIterator import PyDatagramIterator

#N.B. all DOs should be imported here
from direct.distributed import TimeManagerAI

class AIRepository(ConnectionRepository):
    """
    This maintains AI side Distributed Objects.

    Can be used by a VR Studio OTP server or a CMU server to manage
    DistributedObjectAIs. Inherit this to create more DOs.
    """
    notify = DirectNotifyGlobal.directNotify.newCategory("AIRepository")

    def __init__(self, minChannel, maxChannel, serverId=None, dcFileNames=None, dcSuffix='AI', connectMethod=None, threadedNet=None):
        if connectMethod is None:
            connectMethod = self.CM_NATIVE
        ConnectionRepository.__init__(self, connectMethod, base.config, hasOwnerView=False, threadedNet=threadedNet)
        self.setClientDatagram(0)
        self.minChannel = minChannel
        self.maxChannel = maxChannel
        assert maxChannel >= minChannel
        self.dcSuffix = dcSuffix

        self.readDCFile(dcFileNames)
        self.channelAllocator = UniqueIdAllocator(minChannel, maxChannel)
        self.ourChannel = self.allocateChannel()
        self.districtId = 0
        self.accountId = 0
        self.avId = 0

        self.serverId = serverId
        self.doTables = {}
        self.timeManager = TimeManagerAI.TimeManagerAI(self)
        return

    ### AIRepository methods ###

    def handleConnected(self):
        self.registerForChannel(self.ourChannel)

    def handleMsgType(self, msgType, di):
        """Has to be implemented"""
        pass

    def handleDatagram(self, di):
        if self.notify.getDebug():
            self.notify.debug('AIRepository received datagram:')
            di.getDatagram().dumpHex(ostream)

        self.handleMsgType(self.getMsgType(), di)

    def getAvatarIdFromSender(self):
        return self.sender & 0xFFFFFFFF

    def getAvatarExitEvent(self):
        pass

    def writeServerEvent(self, eventType, *args, **kwargs):
        pass

    def stopTrackRequestDeletedDO(self, do):
        pass

    def deallocateChannel(self, channel):
        self.channelAllocator.free(channel)

    def allocateChannel(self):
        return self.channelAllocator.allocate()

    def registerForChannel(self, channel):
        pass

    def unregisterForChannel(self, channel):
        pass

    def addInterest(self, do, zoneId, note='', event=None):
        pass
    def sendSetLocation(self, do, parentId, zoneId):
        pass
    def startMessageBundle(self, name):
        pass

    def sendMessageBundle(self, doId):
        pass

    def sendUpdate(self, do, fieldName, args=[]):
        self.sendUpdateToChannel(do, do.doId, fieldName, args)

    def sendUpdateToChannel(self, do, channel, fieldName, args=[]):
        """Sends a DO field update to an OTP channel."""
        dg = do.dclass.getFieldByName(fieldName).aiFormatUpdate(do.doId, channel, self.ourChannel, args)
        self.send(dg)

    def generateWithRequired(self, do, parentId, zoneId, optionalFields=[]):
        self.generateWithRequiredAndId(do, self.allocateChannel(), parentId, zoneId, optionalFields)

    def generateWithRequiredAndId(self, do, doId, parentId, zoneId, optionalFields=[]):
        """Generates a new DO."""
        do.doId = doId
        self.addDOToTables(do, location=(parentId, zoneId))
        do.sendGenerateWithRequired(self, parentId, zoneId, optionalFields)
        return

    def addDOToTables(self, do, location):
        self.doTables[do] = location

    def requestDelete(self, do):
        del self.doTables[do]

    def replaceMethod(oldFunc, newFunc):
        pass
    def startTrackRequestDeletedDO(self, do):
        pass

    def sendSetZone(self, do, zoneId):
        pass

    def getRender(self, zoneId):
        return
    def getNonCollidableParent(self, zoneId):
        return
    def getParentMgr(self, zoneId):
        return
    def getCollTrav(self, zoneId, *args, **kArgs):
        return

    def getZoneDataStore(self):
        from otp.ai.AIZoneData import AIZoneDataStore
        return AIZoneDataStore()

    def _isValidPlayerLocation(self, parentId, zoneId):
        if zoneId < 1000 and zoneId != 1:
            return False
        else:
            return True

    def incrementPopulation(self):
        """Virtual"""
        pass
    def decrementPopulation(self):
        """Virtual"""
        pass
    def getTrackClsends(self):
        return
    def loadDNAFileAI(self, dnaStore, dnaFileName):
        """Virtual for VR Studio applications"""
        pass
    def queryData(self, accountId, data):
        """NOT IMPLEMENTED"""
    def getDo(self, doId):
        return self.doId2do.get(doId)