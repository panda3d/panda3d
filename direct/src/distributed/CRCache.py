"""CRCache module: contains the CRCache class"""

from direct.directnotify import DirectNotifyGlobal
from . import DistributedObject

class CRCache:
    notify = DirectNotifyGlobal.directNotify.newCategory("CRCache")

    def __init__(self, maxCacheItems=10):
        self.maxCacheItems = maxCacheItems
        self.storedCacheItems = maxCacheItems
        self.dict = {}
        self.fifo = []

    def isEmpty(self):
        return len(self.fifo) == 0

    def flush(self):
        """
        Delete each item in the cache then clear all references to them
        """
        assert self.checkCache()
        CRCache.notify.debug("Flushing the cache")
        # NOTE: delayDeleted objects should no longer get into the cache in the first place
        # give objects a chance to clean themselves up before checking for DelayDelete leaks
        messenger.send('clientCleanup')
        # some of these objects might be holding delayDeletes on others
        # track each object that is delayDeleted after it gets its chance to delete,
        # and check them after all objects have had a chance to delete
        delayDeleted = []
        for distObj in self.dict.values():
            distObj.deleteOrDelay()
            if distObj.getDelayDeleteCount() != 0:
                delayDeleted.append(distObj)
            if distObj.getDelayDeleteCount() <= 0:
                # make sure we're not leaking
                distObj.detectLeaks()
        # now that all objects have had a chance to delete, are there any objects left
        # that are still delayDeleted?
        delayDeleteLeaks = []
        for distObj in delayDeleted:
            if distObj.getDelayDeleteCount() != 0:
                delayDeleteLeaks.append(distObj)
        if len(delayDeleteLeaks):
            s = 'CRCache.flush:'
            for obj in delayDeleteLeaks:
                s += ('\n  could not delete %s (%s), delayDeletes=%s' %
                      (safeRepr(obj), itype(obj), obj.getDelayDeleteNames()))
            self.notify.error(s)
        # Null out all references to the objects so they will get gcd
        self.dict = {}
        self.fifo = []

    def cache(self, distObj):
        # Only distributed objects are allowed in the cache
        assert isinstance(distObj, DistributedObject.DistributedObject)
        assert self.checkCache()
        # Get the doId
        doId = distObj.getDoId()
        # Error check
        success = False
        if doId in self.dict:
            CRCache.notify.warning("Double cache attempted for distObj "
                                   + str(doId))
        else:
            # Call disable on the distObj
            distObj.disableAndAnnounce()

            # Put the distObj in the fifo and the dict
            self.fifo.append(distObj)
            self.dict[doId] = distObj

            success = True

            if len(self.fifo) > self.maxCacheItems:
                # if the cache is full, pop the oldest item
                oldestDistObj = self.fifo.pop(0)
                # and remove it from the dictionary
                del(self.dict[oldestDistObj.getDoId()])
                # and delete it
                oldestDistObj.deleteOrDelay()
                if oldestDistObj.getDelayDeleteCount() <= 0:
                    # make sure we're not leaking
                    oldestDistObj.detectLeaks()

        # Make sure that the fifo and the dictionary are sane
        assert len(self.dict) == len(self.fifo)
        return success

    def retrieve(self, doId):
        assert self.checkCache()
        if doId in self.dict:
            # Find the object
            distObj = self.dict[doId]
            # Remove it from the dictionary
            del(self.dict[doId])
            # Remove it from the fifo
            self.fifo.remove(distObj)
            # return the distObj
            return distObj
        else:
            # If you can't find it, return None
            return None

    def contains(self, doId):
        return doId in self.dict

    def delete(self, doId):
        assert self.checkCache()
        assert doId in self.dict
        # Look it up
        distObj = self.dict[doId]
        # Remove it from the dict and fifo
        del(self.dict[doId])
        self.fifo.remove(distObj)
        # and delete it
        distObj.deleteOrDelay()
        if distObj.getDelayDeleteCount() <= 0:
            # make sure we're not leaking
            distObj.detectLeaks()

    def checkCache(self):
        # For debugging; this verifies that the cache is sensible and
        # returns true if so.
        from panda3d.core import NodePath
        for obj in self.dict.values():
            if isinstance(obj, NodePath):
                assert not obj.isEmpty() and obj.getTopNode() != render.node()
        return 1

    def turnOff(self):
        self.flush()
        self.storedMaxCache = self.maxCacheItems
        self.maxCacheItems = 0

    def turnOn(self):
        self.maxCacheItems = self.storedMaxCache
