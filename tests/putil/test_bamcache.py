from panda3d import core


def test_bamcache_flush_index():
    # We really only have this unit test so that this method is being hit
    # consistently, and not intermittently, to avoid a noisy coverage report.
    cache = core.BamCache()
    cache.flush_index()
