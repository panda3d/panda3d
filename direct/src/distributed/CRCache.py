"""CRCache module: contains the CRCache class"""

from direct.directnotify import DirectNotifyGlobal
import DistributedObject

class CRCache:
    notify = DirectNotifyGlobal.directNotify.newCategory("CRCache")

    def __init__(self, maxCacheItems=10):
        self.maxCacheItems = maxCacheItems
        self.dict = {}
        self.fifo = []
        return None

    def flush(self):
        """
        Delete each item in the cache then clear all references to them
        """
        assert(self.checkCache())
        CRCache.notify.debug("Flushing the cache")
        for distObj in self.dict.values():
            distObj.deleteOrDelay()
        # Null out all references to the objects so they will get gcd
        self.dict = {}
        self.fifo = []

    def cache(self, distObj):
        # Only distributed objects are allowed in the cache
        assert(isinstance(distObj, DistributedObject.DistributedObject))
        assert(self.checkCache())
        # Get the doId
        doId = distObj.getDoId()
        # Error check
        if self.dict.has_key(doId):
            CRCache.notify.warning("Double cache attempted for distObj "
                                   + str(doId))
        else:
            # Call disable on the distObj
            distObj.disableAndAnnounce()
            
            # Put the distObj in the fifo and the dict
            self.fifo.append(distObj)
            self.dict[doId] = distObj
            
            if len(self.fifo) > self.maxCacheItems:
                # if the cache is full, pop the oldest item
                oldestDistObj = self.fifo.pop(0)
                # and remove it from the dictionary
                del(self.dict[oldestDistObj.getDoId()])
                # and delete it
                oldestDistObj.deleteOrDelay()
                
        # Make sure that the fifo and the dictionary are sane
        assert(len(self.dict) == len(self.fifo))
        return None

    def retrieve(self, doId):
        assert(self.checkCache())
        if self.dict.has_key(doId):
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
        return self.dict.has_key(doId)
    
    def delete(self, doId):
        assert(self.checkCache())
        assert(self.dict.has_key(doId))
        # Look it up
        distObj = self.dict[doId]
        # Remove it from the dict and fifo
        del(self.dict[doId])
        self.fifo.remove(distObj)
        # and delete it
        distObj.deleteOrDelay()
        
    def checkCache(self):
        # For debugging; this verifies that the cache is sensible and
        # returns true if so.
        from pandac.PandaModules import NodePath
        for obj in self.dict.values():
            if isinstance(obj, NodePath):
                assert(not obj.isEmpty() and obj.getTopNode() != render.node())
        return 1
