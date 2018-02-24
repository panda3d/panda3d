"""DistributedObjectUD module: contains the DistributedObjectUD class"""

from direct.directnotify.DirectNotifyGlobal import directNotify
from direct.distributed.DistributedObjectBase import DistributedObjectBase
from direct.showbase import PythonUtil
from panda3d.core import *
from panda3d.direct import *
#from PyDatagram import PyDatagram
#from PyDatagramIterator import PyDatagramIterator

class DistributedObjectUD(DistributedObjectBase):
    notify = directNotify.newCategory("DistributedObjectUD")
    QuietZone = 1

    def __init__(self, air):
        try:
            self.DistributedObjectUD_initialized
        except:
            self.DistributedObjectUD_initialized = 1
            DistributedObjectBase.__init__(self, air)

            self.accountName = ''
            # Record the repository
            self.air = air

            # Record our distributed class
            className = self.__class__.__name__
            self.dclass = self.air.dclassesByName[className]
            # init doId pre-allocated flag
            self.__preallocDoId = 0

            # used to track zone changes across the quiet zone
            # NOTE: the quiet zone is defined in OTP, but we need it
            # here.
            self.lastNonQuietZone = None

            self._DOUD_requestedDelete = False

            # These are used to implement beginBarrier().
            self.__nextBarrierContext = 0
            self.__barriers = {}

            self.__generated = False
            # reference count for multiple inheritance
            self.__generates = 0

    # Uncomment if you want to debug DO leaks
    #def __del__(self):
    #    """
    #    For debugging purposes, this just prints out what got deleted
    #    """
    #    print ("Destructing: " + self.__class__.__name__)

    if __debug__:
        def status(self, indent=0):
            """
            print out doId(parentId, zoneId) className
                and conditionally show generated or deleted
            """
            spaces = ' ' * (indent + 2)
            try:
                print("%s%s:" % (' ' * indent, self.__class__.__name__))

                flags = []
                if self.__generated:
                    flags.append("generated")
                if self.air == None:
                    flags.append("deleted")

                flagStr = ""
                if len(flags):
                    flagStr = " (%s)" % (" ".join(flags))

                print("%sfrom DistributedObject doId:%s, parent:%s, zone:%s%s" % (
                    spaces, self.doId, self.parentId, self.zoneId, flagStr))
            except Exception as e:
                print("%serror printing status %s" % (spaces, e))

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
        from DistributedObjectUD more than once.
        """
        self.__generates -= 1
        if self.__generates < 0:
            self.notify.debug('DistributedObjectUD: delete() called more times than generate()')
        if self.__generates == 0:
            # prevent this code from executing multiple times
            if self.air is not None:
                # self.doId may not exist.  The __dict__ syntax works around that.
                assert self.notify.debug('delete(): %s' % (self.__dict__.get("doId")))

                if not self._DOUD_requestedDelete:
                    # this logs every delete that was not requested by us.
                    # TODO: this currently prints warnings for deletes of objects
                    # that we did not create. We need to add a 'locally created'
                    # flag to every object to filter these out.
                    """
                    DistributedObjectUD.notify.warning(
                        'delete() called but requestDelete never called for %s: %s'
                        % (self.__dict__.get('doId'), self.__class__.__name__))
                        """
                    """
                    # print a stack trace so we can detect whether this is the
                    # result of a network msg.
                    # this is slow.
                    from direct.showbase.PythonUtil import StackTrace
                    DistributedObjectUD.notify.warning(
                        'stack trace: %s' % StackTrace())
                        """
                self._DOUD_requestedDelete = False

                # Clean up all the pending barriers.
                for barrier in self.__barriers.values():
                    barrier.cleanup()
                self.__barriers = {}

                # Asad: As per Roger's suggestion, turn off the following block until a solution is
                # Thought out of how to prevent this delete message or to handle this message better
##              if not hasattr(self, "doNotDeallocateChannel"):
##                  if self.air:
##                      self.air.deallocateChannel(self.doId)
##              self.air = None
                self.parentId = None
                self.zoneId = None
                self.__generated = False

    def isDeleted(self):
        """
        Returns true if the object has been deleted,
        or if it is brand new and hasnt yet been generated.
        """
        return self.air == None

    def isGenerated(self):
        """
        Returns true if the object has been generated
        """
        return self.__generated

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
        self.__generated = True

    def postGenerateMessage(self):
        messenger.send(self.uniqueName("generate"), [self])

    def addInterest(self, zoneId, note="", event=None):
        self.air.addInterest(self.getDoId(), zoneId, note, event)

    def b_setLocation(self, parentId, zoneId):
        self.d_setLocation(parentId, zoneId)
        self.setLocation(parentId, zoneId)

    def d_setLocation(self, parentId, zoneId):
        self.air.sendSetLocation(self, parentId, zoneId)

    def setLocation(self, parentId, zoneId):
        self.air.storeObjectLocation(self, parentId, zoneId)

    def getLocation(self):
        try:
            if self.parentId <= 0 and self.zoneId <= 0:
                return None
            # This is a -1 stuffed into a uint32
            if self.parentId == 0xffffffff and self.zoneId == 0xffffffff:
                return None
            return (self.parentId, self.zoneId)
        except AttributeError:
            return None

    def updateRequiredFields(self, dclass, di):
        dclass.receiveUpdateBroadcastRequired(self, di)
        self.announceGenerate()
        self.postGenerateMessage()

    def updateAllRequiredFields(self, dclass, di):
        dclass.receiveUpdateAllRequired(self, di)
        self.announceGenerate()
        self.postGenerateMessage()

    def updateRequiredOtherFields(self, dclass, di):
        dclass.receiveUpdateBroadcastRequired(self, di)
        # Announce generate after updating all the required fields,
        # but before we update the non-required fields.
        self.announceGenerate()
        self.postGenerateMessage()

        dclass.receiveUpdateOther(self, di)

    def updateAllRequiredOtherFields(self, dclass, di):
        dclass.receiveUpdateAllRequired(self, di)
        # Announce generate after updating all the required fields,
        # but before we update the non-required fields.
        self.announceGenerate()
        self.postGenerateMessage()

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

    def getNonCollidableParent(self):
        return self.air.getNonCollidableParent(self.zoneId)

    def getParentMgr(self):
        return self.air.getParentMgr(self.zoneId)

    def getCollTrav(self, *args, **kArgs):
        return self.air.getCollTrav(self.zoneId, *args, **kArgs)

    def sendUpdate(self, fieldName, args = []):
        assert self.notify.debugStateCall(self)
        if self.air:
            self.air.sendUpdate(self, fieldName, args)

    def GetPuppetConnectionChannel(self, doId):
        return doId + (1001 << 32)

    def GetAccountConnectionChannel(self, doId):
        return doId + (1003 << 32)

    def GetAccountIDFromChannelCode(self, channel):
        return channel >> 32

    def GetAvatarIDFromChannelCode(self, channel):
        return channel & 0xffffffff

    def sendUpdateToAvatarId(self, avId, fieldName, args):
        assert self.notify.debugStateCall(self)
        channelId = self.GetPuppetConnectionChannel(avId)
        self.sendUpdateToChannel(channelId, fieldName, args)

    def sendUpdateToAccountId(self, accountId, fieldName, args):
        assert self.notify.debugStateCall(self)
        channelId = self.GetAccountConnectionChannel(accountId)
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
            return self.generateWithRequiredAndId(self.doId, zoneId,
                                                  optionalFields)

        # The repository is the one that really does the work
        parentId = self.air.districtId
        self.parentId = parentId
        self.zoneId = zoneId
        self.air.generateWithRequired(self, parentId, zoneId, optionalFields)
        self.generate()

    # this is a special generate used for estates, or anything else that
    # needs to have a hard coded doId as assigned by the server
    def generateWithRequiredAndId(self, doId, parentId, zoneId, optionalFields=[]):
        assert self.notify.debugStateCall(self)
        # have we already allocated a doId?
        if self.__preallocDoId:
            assert doId == self.__preallocDoId
            self.__preallocDoId = 0

        # The repository is the one that really does the work
        self.air.generateWithRequiredAndId(self, doId, parentId, zoneId, optionalFields)
        ## self.parentId = parentId
        ## self.zoneId = zoneId
        self.generate()
        self.announceGenerate()
        self.postGenerateMessage()

    def generateOtpObject(self, parentId, zoneId, optionalFields=[], doId=None):
        assert self.notify.debugStateCall(self)
        # have we already allocated a doId?
        if self.__preallocDoId:
            assert doId is None or doId == self.__preallocDoId
            doId = self.__preallocDoId
            self.__preallocDoId = 0

        # Assign it an id
        if doId is None:
            self.doId = self.air.allocateChannel()
        else:
            self.doId = doId
        # Put the new DO in the dictionaries
        self.air.addDOToTables(self, location = (parentId, zoneId))
        # Send a generate message
        self.sendGenerateWithRequired(self.air, parentId, zoneId, optionalFields)

        ## assert not hasattr(self, 'parentId') or self.parentId is None
        ## self.parentId = parentId
        ## self.zoneId = zoneId
        self.generate()

    def generate(self):
        """
        Inheritors should put functions that require self.zoneId or
        other networked info in this function.
        """
        assert self.notify.debugStateCall(self)
        self.__generates += 1
        self.air.storeObjectLocation(self, self.parentId, self.zoneId)

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
        # UD.  The input parameter is a dictionary of field names to
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
        self._DOUD_requestedDelete = True

    def taskName(self, taskString):
        return ("%s-%s" % (taskString, self.doId))

    def uniqueName(self, idString):
        return ("%s-%s" % (idString, self.doId))

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

        from otp.ai import Barrier
        context = self.__nextBarrierContext
        # We assume the context number is passed as a uint16.
        self.__nextBarrierContext = (self.__nextBarrierContext + 1) & 0xffff

        assert self.notify.debug('beginBarrier(%s, %s, %s, %s)' % (context, name, avIds, timeout))

        if avIds:
            barrier = Barrier.Barrier(
                name, self.uniqueName(name), avIds, timeout,
                doneFunc = PythonUtil.Functor(
                    self.__barrierCallback, context, callback))
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
        avId = self.air.getAvatarIdFromSender()
        assert self.notify.debug('setBarrierReady(%s, %s)' % (context, avId))
        barrier = self.__barriers.get(context)
        if barrier == None:
            # This may be None if a client was slow and missed an
            # earlier timeout.  Too bad.
            return

        barrier.clear(avId)

    def __barrierCallback(self, context, callback, avIds):
        assert self.notify.debug('barrierCallback(%s, %s)' % (context, avIds))
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

    def execCommand(self, string, mwMgrId, avId, zoneId):
        pass
