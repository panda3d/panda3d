from panda3d import core
import random
import pytest

try:
    import hashlib
except ImportError:
    hashlib = object()
    hashlib.algorithms_available = ()


def test_hashval_hex():
    hex = '%032x' % random.getrandbits(32 * 4)
    val = core.HashVal()
    val.input_hex(core.StringStream(hex.encode('ascii')))
    assert str(val) == hex.lower()


def test_hashval_md5_known():
    known_hashes = {
        'd41d8cd98f00b204e9800998ecf8427e': b'',
        '93b885adfe0da089cdf634904fd59f71': b'\000',
        '3b5d3c7d207e37dceeedd301e35e2e58': b'\000' * 64,
        '202cb962ac59075b964b07152d234b70': b'123',
        '520620de89e220f9b5850cc97cbff46c': b'01234567' * 8,
        'ad32d3ef227a5ebd800a40d4eeaff41f': b'01234567' * 8 + b'a',
    }

    for known, plain in known_hashes.items():
        hv = core.HashVal()
        hv.hash_bytes(plain)
        assert hv.as_hex() == known


@pytest.mark.skipif('md5' not in hashlib.algorithms_available,
                    reason="MD5 algorithm not available in hashlib")
def test_hashval_md5_random():
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


def test_hashval_ordering():
    # Regression test for a HashVal ordering bug which made it unsuitable for
    # use in a map.
    def from_hex(hex):
        val = core.HashVal()
        assert val.set_from_hex(hex)
        return val

    # These three values were mutually circular before:
    # a < b and b < c, yet c < a.
    a = from_hex('00000000000000000000000000000000')
    b = from_hex('60000000000000000000000000000000')
    c = from_hex('c0000000000000000000000000000000')
    assert a < b
    assert b < c
    assert a < c

    # Every word must participate, most significant word first.
    assert from_hex('00000000000000000000000000000001') < \
           from_hex('00000000000000010000000000000000')

    # The ordering must sort arbitrary hashes like 128-bit integers, and
    # compare_to must agree with the comparison operators in both directions.
    hexes = sorted({'%032x' % random.getrandbits(128) for i in range(100)})
    vals = [from_hex(hex) for hex in hexes]
    for val1, val2 in zip(vals, vals[1:]):
        assert val1 < val2
        assert not (val2 < val1)
        assert val1.compare_to(val2) < 0
        assert val2.compare_to(val1) > 0

    for val in vals:
        assert not (val < val)
        assert val.compare_to(val) == 0
