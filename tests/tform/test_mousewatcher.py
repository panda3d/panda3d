from panda3d.core import MouseWatcher, MouseWatcherRegion


def test_mousewatcher_region_add():
    region1 = MouseWatcherRegion("1", 0, 1, 0, 1)
    region2 = MouseWatcherRegion("2", 0, 1, 0, 1)

    mw = MouseWatcher()
    assert len(mw.regions) == 0

    mw.add_region(region1)
    assert len(mw.regions) == 1

    mw.add_region(region2)
    assert len(mw.regions) == 2

    mw.add_region(region1)
    assert len(mw.regions) == 2
