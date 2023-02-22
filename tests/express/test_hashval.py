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
