"""DistributedObjectAI module: contains the DistributedObjectAI class"""

from direct.directnotify.DirectNotifyGlobal import *
from direct.showbase import PythonUtil
from direct.showbase import DirectObject
from pandac.PandaModules import *
from PyDatagram import PyDatagram
from PyDatagramIterator import PyDatagramIterator

class DistributedObjectAI(DirectObject.DirectObject):
    notify = directNotify.newCategory("DistributedObjectAI")
    QuietZone = 1

    def __init__(self, air):
        try:
            self.DistributedObjectAI_initialized
        except:
            self.DistributedObjectAI_initialized = 1

            self.accountName=''
            # Record the repository
            self.air = air
            # Record our parentId and zoneId
            self.__location = None
            
            # Record our distributed class
            className = self.__class__.__name__
            self.dclass = self.air.dclassesByName[className]
            # init doId pre-allocated flag
            self.__preallocDoId = 0

            # used to track zone changes across the quiet zone
            # NOTE: the quiet zone is defined in OTP, but we need it
            # here.
            self.lastNonQuietZone = None

            self._DOAI_requestedDelete = False

            # These are used to implement beginBarrier().
            self.__nextBarrierContext = 0
            self.__barriers = {}

    # Uncomment if you want to debug DO leaks
    #def __del__(self):
    #    """
    #    For debugging purposes, this just prints out what got deleted
    #    """
    #    print ("Destructing: " + self.__class__.__name__)

    def getDeleteEvent(self):
        # this is sent just before we get deleted
        if hasattr(self, 'doId'):
            return 'distObjDelete-%s' % self.doId
        return None

    def sendDeleteEvent(self):
        # this is called just before we get deleted
        delEvent = self.getDeleteEvent()
        if delEvent:
            messenger.send(delEvent)

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

            if not self._DOAI_requestedDelete:
                # this logs every delete that was not requested by us.
                # TODO: this currently prints warnings for deletes of objects
                # that we did not create. We need to add a 'locally created'
                # flag to every object to filter these out.
                """
                DistributedObjectAI.notify.warning(
                    'delete() called but requestDelete never called for %s: %s'
                    % (self.__dict__.get('doId'), self.__class__.__name__))
                    """
                """
                # print a stack trace so we can detect whether this is the
                # result of a network msg.
                # this is slow.
                from direct.showbase.PythonUtil import StackTrace
                DistributedObjectAI.notify.warning(
                    'stack trace: %s' % StackTrace())
                    """
            self._DOAI_requestedDelete = False

            # Clean up all the pending barriers.
            for barrier in self.__barriers.values():
                barrier.cleanup()
            self.__barriers = {}

            if not hasattr(self, "doNotDeallocateChannel"):
                if self.air:
                    self.air.deallocateChannel(self.doId)
            self.air = None
            if hasattr(self, 'parentId'):
                del self.parentId
            del self.zoneId

    def isDeleted(self):
        """
        Returns true if the object has been deleted,
        or if it is brand new and hasn't yet been generated.
        """
        return self.air == None

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

    def announceGenerate(self):
        """
        Called after the object has been generated and all
        of its required fields filled in. Overwrite when needed.
        """
        pass

    if wantOtpServer:
        def addInterest(self, zoneId, note="", event=None):
            self.air.addInterest(self.getDoId(), zoneId, note, event)

        def b_setLocation(self, parentId, zoneId):
            self.d_setLocation(parentId, zoneId)
            self.setLocation(parentId, zoneId)

        def d_setLocation(self, parentId, zoneId):
            self.air.sendSetLocation(self, parentId, zoneId)

        def setLocation(self, parentId, zoneId):
            oldParentId = self.parentId
            oldZoneId = self.zoneId
            if ((oldParentId != parentId) or
                (oldZoneId != zoneId)):
                self.zoneId = zoneId
                self.parentId = parentId
                self.air.changeDOZoneInTables(self, zoneId, oldZoneId)
                messenger.send(self.getZoneChangeEvent(), [zoneId, oldZoneId])
                # if we are not going into the quiet zone, send a 'logical' zone
                # change message
                if zoneId != DistributedObjectAI.QuietZone:
                    lastLogicalZone = oldZoneId
                    if oldZoneId == DistributedObjectAI.QuietZone:
                        lastLogicalZone = self.lastNonQuietZone
                    self.handleLogicalZoneChange(zoneId, lastLogicalZone)
                    self.lastNonQuietZone = zoneId
                #self.air.storeObjectLocation(self.doId, parentId, zoneId)
                self.__location = (parentId, zoneId)

        def getLocation(self):
            return self.__location

    else:
        # NON OTP
        def handleZoneChange(self, newZoneId, oldZoneId):
            self.zoneId = newZoneId
            self.air.changeDOZoneInTables(self, newZoneId, oldZoneId)
            messenger.send(self.getZoneChangeEvent(), [newZoneId, oldZoneId])
            # if we are not going into the quiet zone, send a 'logical' zone change
            # message
            if newZoneId != DistributedObjectAI.QuietZone:
                lastLogicalZone = oldZoneId
                if oldZoneId == DistributedObjectAI.QuietZone:
                    lastLogicalZone = self.lastNonQuietZone
                self.handleLogicalZoneChange(newZoneId, lastLogicalZone)
                self.lastNonQuietZone = newZoneId

    def updateRequiredFields(self, dclass, di):
        dclass.receiveUpdateBroadcastRequired(self, di)
        self.announceGenerate()

    def updateAllRequiredFields(self, dclass, di):
        dclass.receiveUpdateAllRequired(self, di)
        self.announceGenerate()

    def updateRequiredOtherFields(self, dclass, di):
        dclass.receiveUpdateBroadcastRequired(self, di)
        # Announce generate after updating all the required fields,
        # but before we update the non-required fields.
        self.announceGenerate()
        dclass.receiveUpdateOther(self, di)

    def updateAllRequiredOtherFields(self, dclass, di):
        dclass.receiveUpdateAllRequired(self, di)
        # Announce generate after updating all the required fields,
        # but before we update the non-required fields.
        self.announceGenerate()
        dclass.receiveUpdateOther(self, di)

    def sendSetZone(self, zoneId):
        self.air.sendSetZone(self, zoneId)

    def getZoneChangeEvent(self):
        # this event is generated whenever this object changes zones.
        # arguments are newZoneId, oldZoneId
        # includes the quiet zone.
        return 'DOChangeZone-%s' % self.doId
    def getLogicalZoneChangeEvent(self):
        # this event is generated whenever this object changes to a
        # non-quiet-zone zone.
        # arguments are newZoneId, oldZoneId
        # does not include the quiet zone.
        return 'DOLogicalChangeZone-%s' % self.doId

    def handleLogicalZoneChange(self, newZoneId, oldZoneId):
        """this function gets called as if we never go through the
        quiet zone. Note that it is called once you reach the newZone,
        and not at the time that you leave the oldZone."""
        messenger.send(self.getLogicalZoneChangeEvent(),
                       [newZoneId, oldZoneId])

    def getRender(self):
        # note that this will return a different node if we change zones
        return self.air.getRender(self.zoneId)

    def getParentMgr(self):
        return self.air.getParentMgr(self.zoneId)

    def getCollTrav(self):
        return self.air.getCollTrav(self.zoneId)

    def sendUpdate(self, fieldName, args = []):
        assert self.notify.debugStateCall(self)
        if self.air:
            self.air.sendUpdate(self, fieldName, args)

    if wantOtpServer:
        def GetPuppetConnectionChannel(self, doId):
            return doId + ( 1L << 32);

        def GetAccountIDFromChannelCode(self, channel):
            return (channel >> 32)

        def GetAvatarIDFromChannelCode(self, channel):
            return (channel  & 0xffffffffL)

        def sendUpdateToAvatarId(self, avId, fieldName, args):
            assert self.notify.debugStateCall(self)
            channelId = self.GetPuppetConnectionChannel(avId)
            self.sendUpdateToChannel(channelId, fieldName, args)
    else:
        def sendUpdateToAvatarId(self, avId, fieldName, args):
            assert self.notify.debugStateCall(self)
            channelId = avId + 1
            self.sendUpdateToChannel(channelId, fieldName, args)

    def sendUpdateToChannel(self, channelId, fieldName, args):
        assert self.notify.debugStateCall(self)
        if self.air:
            self.air.sendUpdateToChannel(self, channelId, fieldName, args)

    def generateWithRequired(self, zoneId, optionalFields=[]):
        assert self.notify.debugStateCall(self)
        # have we already allocated a doId?
        if self.__preallocDoId:
            self.__preallocDoId = 0
            return self.generateWithRequiredAndId(
                self.doId, zoneId, optionalFields)

        # The repository is the one that really does the work
        self.air.generateWithRequired(self, zoneId, optionalFields)
        if wantOtpServer:
            #HACK:
            if not hasattr(self, 'parentId'):
                parentId = self.air.districtId
                self.parentId = parentId
        self.zoneId = zoneId
        self.generate()

    # this is a special generate used for estates, or anything else that
    # needs to have a hard coded doId as assigned by the server
    def generateWithRequiredAndId(self, doId, zoneId, optionalFields=[]):
        assert self.notify.debugStateCall(self)
        # have we already allocated a doId?
        if self.__preallocDoId:
            assert doId == self.__preallocDoId
            self.__preallocDoId = 0

        # The repository is the one that really does the work
        self.air.generateWithRequiredAndId(self, doId, zoneId, optionalFields)
        if wantOtpServer:
            #HACK:
            if not hasattr(self, 'parentId'):
                parentId = self.air.districtId
                self.parentId = parentId
        self.zoneId = zoneId
        self.generate()

    if wantOtpServer:
        def generateOtpObject(self, parentId, zoneId, optionalFields=[], doId=None):
            assert self.notify.debugStateCall(self)
            # have we already allocated a doId?
            if self.__preallocDoId:
                assert doId is None or doId == self.__preallocDoId
                doId=self.__preallocDoId
                self.__preallocDoId = 0
            self.air.sendGenerateOtpObject(
                    self, parentId, zoneId, optionalFields, doId=doId)
            assert not hasattr(self, 'parentId')
            self.parentId = parentId
            self.zoneId = zoneId
            self.__location = (parentId, zoneId)
            self.generate()

    def generate(self):
        """
        Inheritors should put functions that require self.zoneId or
        other networked info in this function.
        """
        assert self.notify.debugStateCall(self)

    if wantOtpServer:
        def generateInit(self, repository=None):
            """
            First generate (not from cache).
            """
            assert self.notify.debugStateCall(self)

        def generateTargetChannel(self, repository):
            """
            Who to send this to for generate messages
            """
            if hasattr(self, "dbObject"):
                return self.doId
            return repository.serverId

    def sendGenerateWithRequired(self, repository, parentId, zoneId, optionalFields=[]):
        assert self.notify.debugStateCall(self)
        if not wantOtpServer:
            parentId = 0
        # Make the dclass do the hard work
        if not wantOtpServer:
            dg = self.dclass.aiFormatGenerate(
                    self, self.doId, 0, zoneId,
                    repository.districtId,
                    repository.ourChannel,
                    optionalFields)
        else:
            dg = self.dclass.aiFormatGenerate(
                    self, self.doId, parentId, zoneId,
                    #repository.serverId,
                    self.generateTargetChannel(repository),
                    repository.ourChannel,
                    optionalFields)
        repository.send(dg)

    def initFromServerResponse(self, valDict):
        assert self.notify.debugStateCall(self)
        # This is a special method used for estates, etc., which get
        # their fields set from the database indirectly by way of the
        # AI.  The input parameter is a dictionary of field names to
        # datagrams that describes the initial field values from the
        # database.

        dclass = self.dclass
        for key, value in valDict.items():
            # Update the field
            dclass.directUpdate(self, key, value)

    def requestDelete(self):
        assert self.notify.debugStateCall(self)
        if not self.air:
            doId = "none"
            if hasattr(self, "doId"):
                doId = self.doId
            self.notify.warning("Tried to delete a %s (doId %s) that is already deleted" % (self.__class__, doId))
            return
        self.air.requestDelete(self)
        self._DOAI_requestedDelete = True

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
                name, self.uniqueName(name), avIds, timeout,
                doneFunc = PythonUtil.Functor(self.__barrierCallback, context, callback))
            self.__barriers[context] = barrier

            # Send the context number to each involved client.
            self.sendUpdate("setBarrierData", [self.__getBarrierData()])
        else:
            # No avatars; just call the callback immediately.
            callback(avIds)

        return context

    def __getBarrierData(self):
        # Returns the barrier data formatted for sending to the
        # clients.  This lists all of the current outstanding barriers
        # and the avIds waiting for them.
        data = []
        for context, barrier in self.__barriers.items():
            toons = barrier.pendingToons
            if toons:
                data.append((context, barrier.name, toons))
        return data

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
        avId = self.air.GetAvatarIDFromSender()
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

    def isGridParent(self):
        # If this distributed object is a DistributedGrid return 1.  0 by default
        return 0

