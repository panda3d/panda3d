from panda3d.core import BitMask16, BitMask32, BitMask64
from panda3d.core import DoubleBitMaskNative, QuadBitMaskNative
import pickle
import pytest


double_num_bits = DoubleBitMaskNative.get_max_num_bits()
quad_num_bits = QuadBitMaskNative.get_max_num_bits()


def test_bitmask_allon():
    assert BitMask16.all_on().is_all_on()
    assert BitMask32.all_on().is_all_on()
    assert BitMask64.all_on().is_all_on()
    assert DoubleBitMaskNative.all_on().is_all_on()
    assert QuadBitMaskNative.all_on().is_all_on()

    assert DoubleBitMaskNative((1 << double_num_bits) - 1).is_all_on()
    assert QuadBitMaskNative((1 << quad_num_bits) - 1).is_all_on()


def test_bitmask_nonzero():
    assert not BitMask16()
    assert not BitMask32()
    assert not BitMask64()
    assert not DoubleBitMaskNative()
    assert not QuadBitMaskNative()


def test_bitmask_overflow():
    with pytest.raises(OverflowError):
        DoubleBitMaskNative(1 << double_num_bits)

    with pytest.raises(OverflowError):
        QuadBitMaskNative(1 << quad_num_bits)


def test_bitmask_int():
    assert int(BitMask16()) == 0
    assert int(BitMask16(0xfffe)) == 0xfffe

    assert int(BitMask32()) == 0
    assert int(BitMask32(1)) == 1
    assert int(BitMask32(1234567)) == 1234567
    assert int(BitMask32(0x8ff123fe)) == 0x8ff123fe
    assert int(BitMask32(0xffffffff)) == 0xffffffff

    assert int(BitMask64()) == 0
    assert int(BitMask64(0xfffffffffffffffe)) == 0xfffffffffffffffe

    assert int(DoubleBitMaskNative()) == 0
    assert int(DoubleBitMaskNative(1)) == 1
    assert int(DoubleBitMaskNative(1 << (double_num_bits // 2))) == 1 << (double_num_bits // 2)
    assert int(DoubleBitMaskNative(0xffff0001)) == 0xffff0001
    assert int(DoubleBitMaskNative((1 << double_num_bits) - 1)) == (1 << double_num_bits) - 1

    assert int(QuadBitMaskNative()) == 0
    assert int(QuadBitMaskNative(1)) == 1
    assert int(QuadBitMaskNative(1 << (quad_num_bits // 2))) == 1 << (quad_num_bits // 2)
    assert int(QuadBitMaskNative(0xffff0001feff0002)) == 0xffff0001feff0002
    assert int(QuadBitMaskNative((1 << quad_num_bits) - 1)) == (1 << quad_num_bits) - 1


def test_bitmask_pickle():
    assert pickle.loads(pickle.dumps(BitMask16(0), -1)).is_zero()

    mask1 = BitMask16(123)
    data = pickle.dumps(mask1, -1)
    mask2 = pickle.loads(data)
    assert mask1 == mask2
