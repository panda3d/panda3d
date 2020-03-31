import pytest

direct = pytest.importorskip("panda3d.direct")


def test_pack_int8():
    for num in range(-128, 128):
        packer = direct.DCPacker()
        packer.raw_pack_int8(num)
        packer.set_unpack_data(packer.get_bytes())
        assert packer.raw_unpack_int8() == num


def test_pack_uint8():
    for num in range(256):
        packer = direct.DCPacker()
        packer.raw_pack_uint8(num)
        packer.set_unpack_data(packer.get_bytes())
        assert packer.raw_unpack_uint8() == num


def test_pack_int64():
    for num in (0, -1, 0x7fffffff, -0x80000000, 0x7fffffffffffffff, 0x7ffffffffffffffe, -0x8000000000000000, -0x7fffffffffffffff):
        packer = direct.DCPacker()
        packer.raw_pack_int64(num)
        packer.set_unpack_data(packer.get_bytes())
        assert packer.raw_unpack_int64() == num


def test_pack_uint64():
    for num in (0, 1, 0x7fffffff, 0xffffffff, 0x7fffffffffffffff, 0xfffffffffffffffe, 0xffffffffffffffff):
        packer = direct.DCPacker()
        packer.raw_pack_uint64(num)
        packer.set_unpack_data(packer.get_bytes())
        assert packer.raw_unpack_uint64() == num

