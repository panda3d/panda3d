"""DistributedObject module: contains the DistributedObject class"""

from PandaObject import *
from DirectNotifyGlobal import *
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
    """Distributed Object class:"""
    notify = directNotify.newCategory("DistributedObject")

    # A few objects will set neverDisable to 1... Examples are
    # localToon, and anything that lives in the UberZone. This
    # keeps them from being disabled when you change zones,
    # even to the quiet zone.
    neverDisable = 0

    def __init__(self, cr):
        try:
            self.DistributedObject_initialized
        except:
            self.DistributedObject_initialized = 1
            self.cr = cr

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

    #def __del__(self):
    #    """
    #    For debugging purposes, this just prints out what got deleted
    #    """
    #    print ("Destructing: " + self.__class__.__name__ + " id: " + str(self.doId))
    #    PandaObject.__del__(self)

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
            self.notify.debug("delayDeleteCount for doId %s now 0" % (self.doId))
            if self.deleteImminent:
                self.notify.debug("delayDeleteCount for doId %s -- deleteImminent"
                                  % (self.doId))
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
        self.activeState = ESGenerated
        messenger.send(self.uniqueName("generate"), [self])

    def disable(self):
        """
        Inheritors should redefine this to take appropriate action on disable
        """
        assert(self.notify.debug('disable(): %s' % (self.doId)))
        self.activeState = ESDisabled
        self.__callbacks = {}

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
            del self.cr

    def generate(self):
        """
        Inheritors should redefine this to take appropriate action on generate
        """
        assert(self.notify.debug('generate()'))
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
    
    def updateRequiredFields(self, cdc, di):
        for i in cdc.broadcastRequiredCDU:
            i.updateField(cdc, self, di)
        self.announceGenerate()
    
    def updateAllRequiredFields(self, cdc, di):
        for i in cdc.allRequiredCDU:
            i.updateField(cdc, self, di)
        self.announceGenerate()

    def updateRequiredOtherFields(self, cdc, di):
        # First, update the required fields
        for i in cdc.broadcastRequiredCDU:
            i.updateField(cdc, self, di)

        # Announce generate after updating all the required fields,
        # but before we update the non-required fields.
        self.announceGenerate()
        
        # Determine how many other fields there are
        numberOfOtherFields = di.getArg(STUint16)
        # Update each of the other fields
        for i in range(numberOfOtherFields):
            cdc.updateField(self, di)

    def sendUpdate(self, fieldName, args = [], sendToId = None):
        self.cr.sendUpdate(self, fieldName, args, sendToId)

    def taskName(self, taskString):
        return (taskString + "-" + str(self.getDoId()))
    
    def uniqueName(self, idString):
        return (idString + "-" + str(self.getDoId()))

    def isLocal(self):
        # This returns true if the distributed object is "local,"
        # which means the client created it instead of the AI, and it
        # gets some other special handling.  Normally, only the local
        # avatar class overrides this to return true.
        return 0
    


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
        dg = PyDatagram(data)
        dgi = PyDatagramIterator(dg)
        while dgi.getRemainingSize() > 0:
            context = dgi.getUint16()
            numToons = dgi.getUint16()
            for i in range(numToons):
                avId = dgi.getUint32()
                if avId == base.localAvatar.doId:
                    # We found localToon's Id; stop here.
                    self.__barrierContext = context
                    assert(self.notify.debug('setBarrierData(%s)' % (context)))
                    return
                
        assert(self.notify.debug('setBarrierData(%s)' % (None)))
        self.__barrierContext = None
        
    def doneBarrier(self):
        # Tells the AI we have finished handling our task.
        assert(self.notify.debug('doneBarrier(%s)' % (self.__barrierContext)))

        # If this is None, it either means we have called
        # doneBarrier() twice, or we have not received a barrier
        # context from the AI.  I think in either case it's ok to
        # silently ignore the error.
        if self.__barrierContext != None:
            self.sendUpdate("setBarrierReady", [self.__barrierContext])
            self.__barrierContext = None
        
        
