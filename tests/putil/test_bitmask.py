from panda3d import core


def test_bitmask_allon():
    assert core.BitMask16.all_on().is_all_on()
    assert core.BitMask32.all_on().is_all_on()
    assert core.BitMask64.all_on().is_all_on()
    assert core.DoubleBitMaskNative.all_on().is_all_on()
    assert core.QuadBitMaskNative.all_on().is_all_on()
    assert core.BitArray.all_on().is_all_on()
