"""DistributedObject module: contains the DistributedObject class"""

from pandac.PandaModules import *
from direct.directnotify.DirectNotifyGlobal import directNotify
from direct.distributed.DistributedObjectBase import DistributedObjectBase
from direct.showbase.PythonUtil import StackTrace
#from PyDatagram import PyDatagram
#from PyDatagramIterator import PyDatagramIterator

# Values for DistributedObject.activeState

ESNew          = 1
ESDeleted      = 2
ESDisabling    = 3
ESDisabled     = 4  # values here and lower are considered "disabled"
ESGenerating   = 5  # values here and greater are considered "generated"
ESGenerated    = 6

# update this table if the values above change
ESNum2Str = {
    ESNew: 'ESNew',
    ESDeleted: 'ESDeleted',
    ESDisabling: 'ESDisabling',
    ESDisabled: 'ESDisabled',
    ESGenerating: 'ESGenerating',
    ESGenerated: 'ESGenerated',
    }

class DistributedObject(DistributedObjectBase):
    """
    The Distributed Object class is the base class for all network based
    (i.e. distributed) objects.  These will usually (always?) have a
    dclass entry in a *.dc file.
    """
    notify = directNotify.newCategory("DistributedObject")

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
            DistributedObjectBase.__init__(self, cr)

            # Most DistributedObjects are simple and require no real
            # effort to load.  Some, particularly actors, may take
            # some significant time to load; these we can optimize by
            # caching them when they go away instead of necessarily
            # deleting them.  The object should set cacheable to 1 if
            # it needs to be optimized in this way.
            self.setCacheable(0)

            # this is for Toontown only, see toontown.distributed.DelayDeletable
            self._token2delayDeleteName = {}
            self._delayDeleteForceAllow = False
            self._delayDeleted = 0

            # Keep track of our state as a distributed object.  This
            # is only trustworthy if the inheriting class properly
            # calls up the chain for disable() and generate().
            self.activeState = ESNew

            # These are used by getCallbackContext() and doCallbackContext().
            self.__nextContext = 0
            self.__callbacks = {}

            # This is used by doneBarrier().
            self.__barrierContext = None

    if __debug__:
        def status(self, indent=0):
            """
            print out "doId(parentId, zoneId) className
                and conditionally show generated, disabled, neverDisable,
                or cachable"
            """
            spaces=' '*(indent+2)
            try:
                print "%s%s:"%(
                    ' '*indent, self.__class__.__name__)
                print "%sfrom DistributedObject doId:%s, parent:%s, zone:%s"%(
                    spaces,
                    self.doId, self.parentId, self.zoneId),
                flags=[]
                if self.activeState == ESGenerated:
                    flags.append("generated")
                if self.activeState < ESGenerating:
                    flags.append("disabled")
                if self.neverDisable:
                    flags.append("neverDisable")
                if self.cacheable:
                    flags.append("cacheable")
                if len(flags):
                    print "(%s)"%(" ".join(flags),),
                print
            except Exception, e: print "%serror printing status"%(spaces,), e

    def getAutoInterests(self):
        # returns the sub-zones under this object that are automatically
        # opened for us by the server.
        # have we already cached it?
        def _getAutoInterests(cls):
            # returns set of auto-interests for this class and all derived
            # have we already computed this class's autoInterests?
            if 'autoInterests' in cls.__dict__:
                autoInterests = cls.autoInterests
            else:
                autoInterests = set()
                # grab autoInterests from base classes
                for base in cls.__bases__:
                    autoInterests.update(_getAutoInterests(base))
                # grab autoInterests from this class
                if cls.__name__ in self.cr.dclassesByName:
                    dclass = self.cr.dclassesByName[cls.__name__]
                    field = dclass.getFieldByName('AutoInterest')
                    if field is not None:
                        p = DCPacker()
                        p.setUnpackData(field.getDefaultValue())
                        len = p.rawUnpackUint16()/4
                        for i in xrange(len):
                            zone = int(p.rawUnpackUint32())
                            autoInterests.add(zone)
                    autoInterests.update(autoInterests)
                    cls.autoInterests = autoInterests
            return set(autoInterests)
        autoInterests = _getAutoInterests(self.__class__)
        # if the server starts supporting multiple auto-interest per class, this check
        # should be removed
        if len(autoInterests) > 1:
            self.notify.error(
                'only one auto-interest allowed per DC class, %s has %s autoInterests (%s)' %
                (self.dclass.getName(), len(autoInterests), list(autoInterests)))
        _getAutoInterests = None
        return list(autoInterests)

    def setNeverDisable(self, bool):
        assert bool == 1 or bool == 0
        self.neverDisable = bool

    def getNeverDisable(self):
        return self.neverDisable

    def _retrieveCachedData(self):
        # once we know our doId, grab any data that might be stored in the data cache
        # from the last time we were on the client
        if self.cr.doDataCache.hasCachedData(self.doId):
            self._cachedData = self.cr.doDataCache.popCachedData(self.doId)

    def setCachedData(self, name, data):
        assert type(name) == type('')
        # ownership of the data passes to the repository data cache
        self.cr.doDataCache.setCachedData(self.doId, name, data)

    def hasCachedData(self, name):
        assert type(name) == type('')
        if not hasattr(self, '_cachedData'):
            return False
        return name in self._cachedData

    def getCachedData(self, name):
        assert type(name) == type('')
        # ownership of the data passes to the caller of this method
        data = self._cachedData[name]
        del self._cachedData[name]
        return data

    def flushCachedData(self, name):
        assert type(name) == type('')
        # call this to throw out cached data from a previous instantiation
        self._cachedData[name].flush()

    def setCacheable(self, bool):
        assert bool == 1 or bool == 0
        self.cacheable = bool

    def getCacheable(self):
        return self.cacheable

    def deleteOrDelay(self):
        if len(self._token2delayDeleteName) > 0:
            if not self._delayDeleted:
                self._delayDeleted = 1
                # Object is delayDeleted. Clean up DistributedObject state,
                # remove from repository tables, so that we won't crash if
                # another instance of the same object gets generated while
                # this instance is still delayDeleted.
                messenger.send(self.getDelayDeleteEvent())
                self.delayDelete()
                self._deactivateDO()
        else:
            self.disableAnnounceAndDelete()

    def disableAnnounceAndDelete(self):
        self.disableAndAnnounce()
        self.delete()
        self._destroyDO()

    def getDelayDeleteCount(self):
        return len(self._token2delayDeleteName)

    def getDelayDeleteEvent(self):
        return self.uniqueName("delayDelete")

    def getDisableEvent(self):
        return self.uniqueName("disable")

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
            messenger.send(self.getDisableEvent())
            self.disable()
            self.activeState = ESDisabled
            if not self._delayDeleted:
                # if the object is DelayDeleted, _deactivateDO has
                # already been called
                self._deactivateDO()

    def announceGenerate(self):
        """
        Sends a message to the world after the object has been
        generated and all of its required fields filled in.
        """
        assert self.notify.debug('announceGenerate(): %s' % (self.doId))


    def _deactivateDO(self):
        # after this is called, the object is no longer an active DistributedObject
        # and it may be placed in the cache
        if not self.cr:
            # we are going to crash, output the destroyDo stacktrace
            self.notify.warning('self.cr is none in _deactivateDO %d' % self.doId)
            if hasattr(self, 'destroyDoStackTrace'):
                print self.destroyDoStackTrace
        self.__callbacks = {}
        self.cr.closeAutoInterests(self)
        self.setLocation(0,0)
        self.cr.deleteObjectLocation(self, self.parentId, self.zoneId)

    def _destroyDO(self):
        # after this is called, the object is no longer a DistributedObject
        # but may still be used as a DelayDeleted object
        self.destroyDoStackTrace = StackTrace()
        # check for leftover cached data that was not retrieved or flushed by this object
        # this will catch typos in the data name in calls to get/setCachedData
        if hasattr(self, '_cachedData'):
            for name, cachedData in self._cachedData.iteritems():
                self.notify.warning('flushing unretrieved cached data: %s' % name)
                cachedData.flush()
            del self._cachedData
        self.cr = None
        self.dclass = None

    def disable(self):
        """
        Inheritors should redefine this to take appropriate action on disable
        """
        assert self.notify.debug('disable(): %s' % (self.doId))
        pass

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
        assert self.notify.debug('delete(): %s' % (self.doId))
        try:
            self.DistributedObject_deleted
        except:
            self.DistributedObject_deleted = 1

    def generate(self):
        """
        Inheritors should redefine this to take appropriate action on generate
        """
        assert self.notify.debugStateCall(self)
        self.activeState = ESGenerating
        # this has already been set at this point
        #self.cr.storeObjectLocation(self, self.parentId, self.zoneId)
        # semi-hack: we seem to be calling generate() more than once for objects that multiply-inherit
        if not hasattr(self, '_autoInterestHandle'):
            self.cr.openAutoInterests(self)

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


    #This message was moved out of announce generate
    #to avoid ordering issues.  
    def postGenerateMessage(self):
        if self.activeState != ESGenerated:
            self.activeState = ESGenerated
            messenger.send(self.uniqueName("generate"), [self])
            
    def updateRequiredFields(self, dclass, di):
        dclass.receiveUpdateBroadcastRequired(self, di)
        self.announceGenerate()
        self.postGenerateMessage()

    def updateAllRequiredFields(self, dclass, di):
        dclass.receiveUpdateAllRequired(self, di)
        self.announceGenerate()
        self.postGenerateMessage()
        
    def updateRequiredOtherFields(self, dclass, di):
        # First, update the required fields
        dclass.receiveUpdateBroadcastRequired(self, di)

        # Announce generate after updating all the required fields,
        # but before we update the non-required fields.
        self.announceGenerate()
        self.postGenerateMessage()
        
        dclass.receiveUpdateOther(self, di)

    def sendUpdate(self, fieldName, args = [], sendToId = None):
        if self.cr:
            dg = self.dclass.clientFormatUpdate(
                fieldName, sendToId or self.doId, args)
            self.cr.send(dg)
        else:
            assert self.notify.error("sendUpdate failed, because self.cr is not set")

    def sendDisableMsg(self):
        self.cr.sendDisableMsg(self.doId)

    def sendDeleteMsg(self):
        self.cr.sendDeleteMsg(self.doId)

    def taskName(self, taskString):
        return ("%s-%s" % (taskString, self.doId))

    def uniqueName(self, idString):
        return ("%s-%s" % (idString, self.doId))

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
                assert self.notify.debug('setBarrierData(%s, %s)' % (context, name))
                return

        assert self.notify.debug('setBarrierData(%s)' % (None))
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
                assert self.notify.debug('doneBarrier(%s, %s)' % (context, aiName))
                self.sendUpdate("setBarrierReady", [context])
                self.__barrierContext = None
            else:
                assert self.notify.debug('doneBarrier(%s) ignored; current barrier is %s' % (name, aiName))
        else:
            assert self.notify.debug('doneBarrier(%s) ignored; no active barrier.' % (name))

    def addInterest(self, zoneId, note="", event=None):
        return self.cr.addInterest(self.getDoId(), zoneId, note, event)

    def removeInterest(self, handle, event=None):
        return self.cr.removeInterest(handle, event)

    def b_setLocation(self, parentId, zoneId):
        self.d_setLocation(parentId, zoneId)
        self.setLocation(parentId, zoneId)

    def d_setLocation(self, parentId, zoneId):
        self.cr.sendSetLocation(self.doId, parentId, zoneId)

    def setLocation(self, parentId, zoneId):
        self.cr.storeObjectLocation(self, parentId, zoneId)

    def getLocation(self):
        try:
            if self.parentId == 0 and self.zoneId == 0:
                return None
            # This is a -1 stuffed into a uint32
            if self.parentId == 0xffffffff and self.zoneId == 0xffffffff:
                return None
            return (self.parentId, self.zoneId)
        except AttributeError:
            return None

    def getParentObj(self):
        if self.parentId is None:
            return None
        return self.cr.doId2do.get(self.parentId)

    def isLocal(self):
        # This returns true if the distributed object is "local,"
        # which means the client created it instead of the AI, and it
        # gets some other special handling.  Normally, only the local
        # avatar class overrides this to return true.
        return self.cr and self.cr.isLocalId(self.doId)

    def updateZone(self, zoneId):
        self.cr.sendUpdateZone(self, zoneId)

    def isGridParent(self):
        # If this distributed object is a DistributedGrid return 1.  0 by default
        return 0

    def execCommand(self, string, mwMgrId, avId, zoneId):
        pass
