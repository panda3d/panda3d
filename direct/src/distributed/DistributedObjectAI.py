"""DistributedObjectAI module: contains the DistributedObjectAI class"""

from direct.directnotify.DirectNotifyGlobal import *
from direct.showbase import PythonUtil
from direct.showbase import DirectObject
from pandac.PandaModules import *
from PyDatagram import PyDatagram
from PyDatagramIterator import PyDatagramIterator

class DistributedObjectAI(DirectObject.DirectObject):
    """Distributed Object class:"""
    
    notify = directNotify.newCategory("DistributedObjectAI")
    
    def __init__(self, air):
        try:
            self.DistributedObjectAI_initialized
        except:
            self.DistributedObjectAI_initialized = 1
            # Record the repository
            self.air = air
            # Record our distributed class
            className = self.__class__.__name__
            self.dclass = self.air.dclassesByName[className]
            # init doId pre-allocated flag
            self.__preallocDoId = 0

            # These are used to implement beginBarrier().
            self.__nextBarrierContext = 0
            self.__barriers = {}

    # Uncomment if you want to debug DO leaks
    #def __del__(self):
    #    """
    #    For debugging purposes, this just prints out what got deleted
    #    """
    #    print ("Destructing: " + self.__class__.__name__)

    def delete(self):
        """
        Inheritors should redefine this to take appropriate action on delete
        Note that this may be called multiple times if a class inherits
        from DistributedObjectAI more than once.
        """
        # prevent this code from executing multiple times
        if self.air is not None:
            # self.doId may not exist.  The __dict__ syntax works around that.
            assert(self.notify.debug('delete(): %s' % (self.__dict__.get("doId"))))
            # Clean up all the pending barriers.
            for barrier in self.__barriers.values():
                barrier.cleanup()
            self.__barriers = {}

            if not hasattr(self, "doNotDeallocateChannel"):
                if self.air:
                    self.air.deallocateChannel(self.doId)
            self.air = None
            del self.zoneId

    def isDeleted(self):
        """
        Returns true if the object has been deleted,
        or if it is brand new and hasn't yet been generated.
        """
        return (self.air == None)

    def isGenerated(self):
        """
        Returns true if the object has been generated
        """
        return hasattr(self, 'zoneId')
    
    def getDoId(self):
        """
        Return the distributed object id
        """
        return self.doId

    def preAllocateDoId(self):
        """
        objects that need to have a doId before they are generated
        can call this to pre-allocate a doId for the object
        """
        assert not self.__preallocDoId
        self.doId = self.air.allocateChannel()
        self.__preallocDoId = 1
    
    def updateRequiredFields(self, dclass, di):
        dclass.receiveUpdateBroadcastRequired(self, di)
    
    def updateAllRequiredFields(self, dclass, di):
        dclass.receiveUpdateAllRequired(self, di)

    def updateRequiredOtherFields(self, dclass, di):
        dclass.receiveUpdateBroadcastRequired(self, di)
        dclass.receiveUpdateOther(self, di)

    def updateAllRequiredOtherFields(self, dclass, di):
        dclass.receiveUpdateAllRequired(self, di)
        dclass.receiveUpdateOther(self, di)

    def getZoneChangeEvent(self):
        return 'DOChangeZone-%s' % self.doId
    
    def handleZoneChange(self, newZoneId, oldZoneId):
        assert oldZoneId == self.zoneId
        self.zoneId = newZoneId
        self.air.changeDOZoneInTables(self, newZoneId, oldZoneId)
        messenger.send(self.getZoneChangeEvent(), [newZoneId, oldZoneId])

    def sendUpdate(self, fieldName, args = []):
        if self.air:
            self.air.sendUpdate(self, fieldName, args)

    def sendUpdateToAvatarId(self, avId, fieldName, args):
        channelId = avId + 1
        self.sendUpdateToChannel(channelId, fieldName, args)

    def sendUpdateToChannel(self, channelId, fieldName, args):
        if self.air:
            self.air.sendUpdateToChannel(self, channelId, fieldName, args)

    def generateWithRequired(self, zoneId, optionalFields=[]):
        # have we already allocated a doId?
        if self.__preallocDoId:
            self.__preallocDoId = 0
            return self.generateWithRequiredAndId(
                self.doId, zoneId, optionalFields)
            
        # The repository is the one that really does the work
        self.air.generateWithRequired(self, zoneId, optionalFields)
        self.zoneId = zoneId
        self.generate()

    # this is a special generate used for estates, or anything else that
    # needs to have a hard coded doId as assigned by the server
    def generateWithRequiredAndId(self, doId, zoneId, optionalFields=[]):
        # have we already allocated a doId?
        if self.__preallocDoId:
            self.__preallocDoId = 0
            assert doId == self.doId

        # The repository is the one that really does the work
        self.air.generateWithRequiredAndId(self, doId, zoneId, optionalFields)
        self.zoneId = zoneId
        self.generate()

    def generate(self):
        # Inheritors should put functions that require self.zoneId or
        # other networked info in this function.
        assert(self.notify.debug('generate(): %s' % (self.doId)))
        pass

    def sendGenerateWithRequired(self, repository, zoneId, optionalFields=[]):
        # Make the dclass do the hard work
        dg = self.dclass.aiFormatGenerate(self, self.doId, zoneId,
                                          repository.districtId,
                                          repository.ourChannel,
                                          optionalFields)
        repository.send(dg)
        self.zoneId = zoneId
            
    def initFromServerResponse(self, valDict):
        # This is a special method used for estates, etc., which get
        # their fields set from the database indirectly by way of the
        # AI.  The input parameter is a dictionary of field names to
        # datagrams that describes the initial field values from the
        # database.
        assert(self.notify.debug("initFromServerResponse(%s)" % (valDict.keys(),)))

        dclass = self.dclass
        for key, value in valDict.items():
            # Update the field
            dclass.directUpdate(self, key, value)

    def requestDelete(self):
        if not self.air:
            doId = "none"
            if hasattr(self, "doId"):
                doId = self.doId
            self.notify.warning("Tried to delete a %s (doId %s) that is already deleted" % (self.__class__, doId))
            return
        self.air.requestDelete(self)

    def taskName(self, taskString):
        return (taskString + "-" + str(self.getDoId()))

    def uniqueName(self, idString):
        return (idString + "-" + str(self.getDoId()))
    
    def validate(self, avId, bool, msg):
        if not bool:
            self.air.writeServerEvent('suspicious', avId, msg)
            self.notify.warning('validate error: avId: %s -- %s' % (avId, msg))
        return bool

    def beginBarrier(self, name, avIds, timeout, callback):
        # Begins waiting for a set of avatars.  When all avatars in
        # the list have reported back in or the callback has expired,
        # calls the indicated callback with the list of toons that
        # made it through.  There may be multiple barriers waiting
        # simultaneously on different lists of avatars, although they
        # should have different names.
        
        from toontown.ai import ToonBarrier
        context = self.__nextBarrierContext
        # We assume the context number is passed as a uint16.
        self.__nextBarrierContext = (self.__nextBarrierContext + 1) & 0xffff

        assert(self.notify.debug('beginBarrier(%s, %s, %s, %s)' % (context, name, avIds, timeout)))

        if avIds:
            barrier = ToonBarrier.ToonBarrier(
                self.uniqueName(name), avIds, timeout,
                doneFunc = PythonUtil.Functor(self.__barrierCallback, context, callback))
            self.__barriers[context] = barrier

            # Send the context number to each involved client.
            self.sendUpdate("setBarrierData", [self.__getBarrierData()])
        else:
            # No avatars; just call the callback immediately.
            callback(avIds)

        return context

    def __getBarrierData(self):
        # Returns the barrier data formatted as a blob for sending to
        # the clients.  This lists all of the current outstanding
        # barriers and the avIds waiting for them.
        dg = PyDatagram()
        for context, barrier in self.__barriers.items():
            toons = barrier.pendingToons
            if toons:
                dg.addUint16(context)
                dg.addUint16(len(toons))
                for avId in toons:
                    dg.addUint32(avId)
        return dg.getMessage()

    def ignoreBarrier(self, context):
        # Aborts a previously-set barrier.  The context is the return
        # value from the previous call to beginBarrier().
        barrier = self.__barriers.get(context)
        if barrier:
            barrier.cleanup()
            del self.__barriers[context]

    def setBarrierReady(self, context):
        # Generated by the clients to check in after a beginBarrier()
        # call.
        avId = self.air.msgSender
        assert(self.notify.debug('setBarrierReady(%s, %s)' % (context, avId)))
        barrier = self.__barriers.get(context)
        if barrier == None:
            # This may be None if a client was slow and missed an
            # earlier timeout.  Too bad.
            return

        barrier.clear(avId)

    def __barrierCallback(self, context, callback, avIds):
        assert(self.notify.debug('barrierCallback(%s, %s)' % (context, avIds)))
        # The callback that is generated when a barrier is completed.
        barrier = self.__barriers.get(context)
        if barrier:
            barrier.cleanup()
            del self.__barriers[context]
            callback(avIds)
        else:
            self.notify.warning("Unexpected completion from barrier %s" % (context))
        
        
