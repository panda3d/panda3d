"""DistributedObject module: contains the DistributedObject class"""

from direct.showbase.PandaObject import *
from direct.directnotify import DirectNotifyGlobal
from PyDatagram import PyDatagram
from PyDatagramIterator import PyDatagramIterator

# Values for DistributedObject.activeState

ESNew          = 1
ESDeleted      = 2
ESDisabling    = 3
ESDisabled     = 4  # values here and lower are considered "disabled"
ESGenerating   = 5  # values here and greater are considered "generated"
ESGenerated    = 6

class DistributedObject(PandaObject):
    """
    The Distributed Object class is the base class for all network based
    (i.e. distributed) objects.  These will usually (always?) have a
    dclass entry in a *.dc file.
    """
    notify = DirectNotifyGlobal.directNotify.newCategory("DistributedObject")

    # A few objects will set neverDisable to 1... Examples are
    # localToon, and anything that lives in the UberZone. This
    # keeps them from being disabled when you change zones,
    # even to the quiet zone.
    neverDisable = 0

    def __init__(self, cr):
        assert self.notify.debugStateCall(self)
        try:
            self.DistributedObject_initialized
        except:
            self.DistributedObject_initialized = 1
            self.cr = cr
            if wantOtpServer:
                # Location stores the parentId, zoneId of this object
                self.__location = (None, None)

            # Most DistributedObjects are simple and require no real
            # effort to load.  Some, particularly actors, may take
            # some significant time to load; these we can optimize by
            # caching them when they go away instead of necessarily
            # deleting them.  The object should set cacheable to 1 if
            # it needs to be optimized in this way.
            self.setCacheable(0)

            # This count tells whether the object can be deleted right away,
            # or not.
            self.delayDeleteCount = 0
            # This flag tells whether a delete has been requested on this
            # object.
            self.deleteImminent = 0

            # Keep track of our state as a distributed object.  This
            # is only trustworthy if the inheriting class properly
            # calls up the chain for disable() and generate().
            self.activeState = ESNew

            # These are used by getCallbackContext() and doCallbackContext().
            self.__nextContext = 0
            self.__callbacks = {}

            # This is used by doneBarrier().
            self.__barrierContext = None

            #zone of the distributed object, default to 0
            self.zone = 0

    def setNeverDisable(self, bool):
        assert((bool == 1) or (bool == 0))
        self.neverDisable = bool

    def getNeverDisable(self):
        return self.neverDisable

    def setCacheable(self, bool):
        assert((bool == 1) or (bool == 0))
        self.cacheable = bool
        return None

    def getCacheable(self):
        return self.cacheable

    def deleteOrDelay(self):
        if self.delayDeleteCount > 0:
            self.deleteImminent = 1
        else:
            self.disableAnnounceAndDelete()

    def delayDelete(self, flag):
        # Flag should be 0 or 1, meaning increment or decrement count
        # Also see DelayDelete.py

        if (flag == 1):
            self.delayDeleteCount += 1
        elif (flag == 0):
            self.delayDeleteCount -= 1
        else:
            self.notify.error("Invalid flag passed to delayDelete: " + str(flag))

        if (self.delayDeleteCount < 0):
            self.notify.error("Somebody decremented delayDelete for doId %s without incrementing"
                              % (self.doId))
        elif (self.delayDeleteCount == 0):
            assert(self.notify.debug("delayDeleteCount for doId %s now 0"
                                     % (self.doId)))
            if self.deleteImminent:
                assert(self.notify.debug("delayDeleteCount for doId %s -- deleteImminent"
                                         % (self.doId)))
                self.disableAnnounceAndDelete()
        else:
            self.notify.debug("delayDeleteCount for doId %s now %s"
                              % (self.doId, self.delayDeleteCount))

        # Return the count just for kicks
        return self.delayDeleteCount

    def disableAnnounceAndDelete(self):
        self.disableAndAnnounce()
        self.delete()

    def disableAndAnnounce(self):
        """
        Inheritors should *not* redefine this function.
        """
        # We must send the disable announce message *before* we
        # actually disable the object.  That way, the various cleanup
        # tasks can run first and take care of restoring the object to
        # a normal, nondisabled state; and *then* the disable function
        # can properly disable it (for instance, by parenting it to
        # hidden).
        if self.activeState != ESDisabled:
            self.activeState = ESDisabling
            messenger.send(self.uniqueName("disable"))
            self.disable()

    def announceGenerate(self):
        """
        Sends a message to the world after the object has been
        generated and all of its required fields filled in.
        """
        assert(self.notify.debug('announceGenerate(): %s' % (self.doId)))
        if self.activeState != ESGenerated:
            self.activeState = ESGenerated
            messenger.send(self.uniqueName("generate"), [self])

    def disable(self):
        """
        Inheritors should redefine this to take appropriate action on disable
        """
        assert(self.notify.debug('disable(): %s' % (self.doId)))
        if self.activeState != ESDisabled:
            self.activeState = ESDisabled
            self.__callbacks = {}
            if wantOtpServer:
                #self.cr.deleteObjectLocation(self.doId, self.__location[0], self.__location[1])
                self.__location = (None, None)
                # TODO: disable my children

    def isDisabled(self):
        """
        Returns true if the object has been disabled and/or deleted,
        or if it is brand new and hasn't yet been generated.
        """
        return (self.activeState < ESGenerating)

    def isGenerated(self):
        """
        Returns true if the object has been fully generated by now,
        and not yet disabled.
        """
        assert self.notify.debugStateCall(self)
        return (self.activeState == ESGenerated)

    def delete(self):
        """
        Inheritors should redefine this to take appropriate action on delete
        """
        assert(self.notify.debug('delete(): %s' % (self.doId)))
        try:
            self.DistributedObject_deleted
        except:
            self.DistributedObject_deleted = 1
            self.cr = None
            self.dclass = None

    def generate(self):
        """
        Inheritors should redefine this to take appropriate action on generate
        """
        assert self.notify.debugStateCall(self)
        self.activeState = ESGenerating

    def generateInit(self):
        """
        This method is called when the DistributedObject is first introduced
        to the world... Not when it is pulled from the cache.
        """
        self.activeState = ESGenerating

    def getDoId(self):
        """
        Return the distributed object id
        """
        return self.doId

    def updateRequiredFields(self, dclass, di):
        dclass.receiveUpdateBroadcastRequired(self, di)
        self.announceGenerate()

    def updateAllRequiredFields(self, dclass, di):
        dclass.receiveUpdateAllRequired(self, di)
        self.announceGenerate()

    def updateRequiredOtherFields(self, dclass, di):
        # First, update the required fields
        dclass.receiveUpdateBroadcastRequired(self, di)

        # Announce generate after updating all the required fields,
        # but before we update the non-required fields.
        self.announceGenerate()

        dclass.receiveUpdateOther(self, di)

    def sendUpdate(self, fieldName, args = [], sendToId = None):
        if self.cr:
            self.cr.sendUpdate(self, fieldName, args, sendToId)

    def sendDisableMsg(self):
        self.cr.sendDisableMsg(self.doId)

    def sendDeleteMsg(self):
        self.cr.sendDeleteMsg(self.doId)

    def taskName(self, taskString):
        return (taskString + "-" + str(self.getDoId()))

    def uniqueName(self, idString):
        return (idString + "-" + str(self.getDoId()))

    def getCallbackContext(self, callback, extraArgs = []):
        # Some objects implement a back-and-forth handshake operation
        # with the AI via an arbitrary context number.  This method
        # (coupled with doCallbackContext(), below) maps a Python
        # callback onto that context number so that client code may
        # easily call the method and wait for a callback, rather than
        # having to negotiate context numbers.

        # This method generates a new context number and stores the
        # callback so that it may later be called when the response is
        # returned.

        # This is intended to be called within derivations of
        # DistributedObject, not directly by other objects.

        context = self.__nextContext
        self.__callbacks[context] = (callback, extraArgs)
        # We assume the context number is passed as a uint16.
        self.__nextContext = (self.__nextContext + 1) & 0xffff

        return context

    def getCurrentContexts(self):
        # Returns a list of the currently outstanding contexts created
        # by getCallbackContext().
        return self.__callbacks.keys()

    def getCallback(self, context):
        # Returns the callback that was passed in to the previous
        # call to getCallbackContext.
        return self.__callbacks[context][0]

    def getCallbackArgs(self, context):
        # Returns the extraArgs that were passed in to the previous
        # call to getCallbackContext.
        return self.__callbacks[context][1]

    def doCallbackContext(self, context, args):
        # This is called after the AI has responded to the message
        # sent via getCallbackContext(), above.  The context number is
        # looked up in the table and the associated callback is
        # issued.

        # This is intended to be called within derivations of
        # DistributedObject, not directly by other objects.

        tuple = self.__callbacks.get(context)
        if tuple:
            callback, extraArgs = tuple
            completeArgs = args + extraArgs
            if callback != None:
                callback(*completeArgs)
            del self.__callbacks[context]
        else:
            self.notify.warning("Got unexpected context from AI: %s" % (context))

    def setBarrierData(self, data):
        # This message is sent by the AI to tell us the barriers and
        # avIds for which the AI is currently waiting.  The client
        # needs to look up its pending context in the table (and
        # ignore the other contexts).  When the client is done
        # handling whatever it should handle in its current state, it
        # should call doneBarrier(), which will send the context
        # number back to the AI.
        for context, name, avIds in data:
            if base.localAvatar.doId in avIds:
                # We found localToon's id; stop here.
                self.__barrierContext = (context, name)
                assert(self.notify.debug('setBarrierData(%s, %s)' % (context, name)))
                return

        assert(self.notify.debug('setBarrierData(%s)' % (None)))
        self.__barrierContext = None

    def doneBarrier(self, name = None):
        # Tells the AI we have finished handling our task.  If the
        # optional name parameter is specified, it must match the
        # barrier name specified on the AI, or the barrier is ignored.
        # This is used to ensure we are not clearing the wrong
        # barrier.

        # If this is None, it either means we have called
        # doneBarrier() twice, or we have not received a barrier
        # context from the AI.  I think in either case it's ok to
        # silently ignore the error.
        if self.__barrierContext != None:
            context, aiName = self.__barrierContext
            if name == None or name == aiName:
                assert(self.notify.debug('doneBarrier(%s, %s)' % (context, aiName)))
                self.sendUpdate("setBarrierReady", [context])
                self.__barrierContext = None
            else:
                assert(self.notify.debug('doneBarrier(%s) ignored; current barrier is %s' % (name, aiName)))
        else:
            assert(self.notify.debug('doneBarrier(%s) ignored; no active barrier.' % (name)))

    if wantOtpServer:
        def addInterest(self, zoneId, note="", event=None):
            self.cr.addInterest(self.getDoId(), zoneId, note, event)

        def setLocation(self, parentId, zoneId):
            # The store must run first so we know the old location
            #self.cr.storeObjectLocation(self.doId, parentId, zoneId)
            self.__location = (parentId, zoneId)
            
        def getLocation(self):
            return self.__location

    def isLocal(self):
        # This returns true if the distributed object is "local,"
        # which means the client created it instead of the AI, and it
        # gets some other special handling.  Normally, only the local
        # avatar class overrides this to return true.
        return self.cr and self.cr.isLocalId(self.doId)

    def updateZone(self, zoneId):
        self.cr.sendUpdateZone(self, zoneId)

