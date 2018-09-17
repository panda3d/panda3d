from direct.distributed.CachedDOData import CachedDOData
from panda3d.core import ConfigVariableInt


__all__ = ["CRDataCache"]

class CRDataCache:
    # Stores cached data for DistributedObjects between instantiations on the client

    def __init__(self):
        self._doId2name2data = {}
        # maximum # of objects we will cache data for
        self._size = ConfigVariableInt('crdatacache-size', 10).getValue()
        assert self._size > 0
        # used to preserve the cache size
        self._junkIndex = 0

    def destroy(self):
        del self._doId2name2data

    def setCachedData(self, doId, name, data):
        # stores a set of named data for a DistributedObject
        assert isinstance(data, CachedDOData)
        if len(self._doId2name2data) >= self._size:
            # cache is full, throw out a random doId's data
            if self._junkIndex >= len(self._doId2name2data):
                self._junkIndex = 0
            junkDoId = list(self._doId2name2data.keys())[self._junkIndex]
            self._junkIndex += 1
            for name in self._doId2name2data[junkDoId]:
                self._doId2name2data[junkDoId][name].flush()
            del self._doId2name2data[junkDoId]

        self._doId2name2data.setdefault(doId, {})
        cachedData = self._doId2name2data[doId].get(name)
        if cachedData:
            cachedData.flush()
            cachedData.destroy()
        self._doId2name2data[doId][name] = data

    def hasCachedData(self, doId):
        return doId in self._doId2name2data

    def popCachedData(self, doId):
        # retrieves all cached data for a DistributedObject and removes it from the cache
        data = self._doId2name2data[doId]
        del self._doId2name2data[doId]
        return data

    def flush(self):
        # get rid of all cached data
        for doId in self._doId2name2data:
            for name in self._doId2name2data[doId]:
                self._doId2name2data[doId][name].flush()
        self._doId2name2data = {}

    if __debug__:
        def _startMemLeakCheck(self):
            self._len = len(self._doId2name2data)

        def _stopMemLeakCheck(self):
            del self._len

        def _checkMemLeaks(self):
            assert self._len == len(self._doId2name2data)

if __debug__:
    class TestCachedData(CachedDOData):
        def __init__(self):
            CachedDOData.__init__(self)
            self._destroyed = False
            self._flushed = False
        def destroy(self):
            CachedDOData.destroy(self)
            self._destroyed = True
        def flush(self):
            CachedDOData.flush(self)
            self._flushed = True

    dc = CRDataCache()
    dc._startMemLeakCheck()

    cd = CachedDOData()
    cd.foo = 34
    dc.setCachedData(1, 'testCachedData', cd)
    del cd
    cd = CachedDOData()
    cd.bar = 45
    dc.setCachedData(1, 'testCachedData2', cd)
    del cd
    assert dc.hasCachedData(1)
    assert dc.hasCachedData(1)
    assert not dc.hasCachedData(2)
    # data is dict of dataName->data
    data = dc.popCachedData(1)
    assert len(data) == 2
    assert 'testCachedData' in data
    assert 'testCachedData2' in data
    assert data['testCachedData'].foo == 34
    assert data['testCachedData2'].bar == 45
    for cd in data.values():
        cd.flush()
    del data
    dc._checkMemLeaks()

    cd = CachedDOData()
    cd.bar = 1234
    dc.setCachedData(43, 'testCachedData2', cd)
    del cd
    assert dc.hasCachedData(43)
    dc.flush()
    dc._checkMemLeaks()

    dc._stopMemLeakCheck()
    dc.destroy()
    del dc

