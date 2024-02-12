from direct.distributed.CachedDOData import CachedDOData
from direct.distributed.CRDataCache import CRDataCache


def test_CRDataCache():
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
