from panda3d import core
import random
import hashlib
import pytest


def test_hashval_hex():
    hex = '%032x' % random.getrandbits(32 * 4)
    val = core.HashVal()
    val.input_hex(core.StringStream(hex.encode('ascii')))
    assert str(val) == hex.lower()


@pytest.mark.skipif('md5' not in hashlib.algorithms_available,
                    reason="MD5 algorithm not available in hashlib")
def test_hashval_md5():
    data = bytearray()

    for i in range(2500):
        control = hashlib.md5(data).hexdigest()

        # Test hash_bytes
        hv = core.HashVal()
        hv.hash_bytes(bytes(data))
        assert hv.as_hex() == control

        # Test hash_stream
        hv = core.HashVal()
        result = hv.hash_stream(core.StringStream(data))
        assert result
        assert hv.as_hex() == control

        data.append(random.randint(0, 255))
