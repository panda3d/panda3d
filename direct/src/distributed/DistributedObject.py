"""DistributedObject module: contains the DistributedObject class"""

from PandaObject import *
from DirectNotifyGlobal import *

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
            
        return None

    #def __del__(self):
    #    """
    #    For debugging purposes, this just prints out what got deleted
    #    """
    #    print ("Destructing: " + self.__class__.__name__ + " id: " + str(self.doId))
    #    PandaObject.__del__(self)

    def setNeverDisable(self, bool):
        assert((bool == 1) or (bool == 0))
        self.neverDisable = bool
        return None

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
        return None

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
        return None

    def disableAndAnnounce(self):
        """disableAndAnnounce(self)
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
        return None

    def announceGenerate(self):
        """announceGenerate(self)
        Sends a message to the world after the object has been
        generated and all of its required fields filled in.
        """
        self.activeState = ESGenerated
        messenger.send(self.uniqueName("generate"), [self])

    def disable(self):
        """disable(self)
        Inheritors should redefine this to take appropriate action on disable
        """
        self.activeState = ESDisabled

    def isDisabled(self):
        """isDisabled(self)
        Returns true if the object has been disabled and/or deleted,
        or if it is brand new and hasn't yet been generated.
        """
        return (self.activeState < ESGenerating)

    def delete(self):
        """delete(self)
        Inheritors should redefine this to take appropriate action on delete
        """
        try:
            self.DistributedObject_deleted
        except:
            self.DistributedObject_deleted = 1
            del self.cr
            return

    def generate(self):
        """generate(self)
        Inheritors should redefine this to take appropriate action on generate
        """
        self.activeState = ESGenerating

    def generateInit(self):
        """generateInit(self)
        This method is called when the DistributedObject is first introduced
        to the world... Not when it is pulled from the cache.
        """
        self.activeState = ESGenerating
    
    def getDoId(self):
        """getDoId(self)
        Return the distributed object id
        """
        return self.doId
    
    def updateRequiredFields(self, cdc, di):
        for i in cdc.broadcastRequiredCDU:
            i.updateField(cdc, self, di)
    
    def updateAllRequiredFields(self, cdc, di):
        for i in cdc.allRequiredCDU:
            i.updateField(cdc, self, di)

    def updateRequiredOtherFields(self, cdc, di):
        # First, update the required fields
        for i in cdc.broadcastRequiredCDU:
            i.updateField(cdc, self, di)
        # Determine how many other fields there are
        numberOfOtherFields = di.getArg(STUint16)
        # Update each of the other fields
        for i in range(numberOfOtherFields):
            cdc.updateField(self, di)
        return None

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
    


