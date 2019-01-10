from panda3d import core
import random


def test_hashval_hex():
    hex = '%032x' % random.getrandbits(32 * 4)
    val = core.HashVal()
    val.input_hex(core.StringStream(hex.encode('ascii')))
    assert str(val) == hex.lower()
