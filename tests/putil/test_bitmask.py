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


def test_bitmask_overflow():
    with pytest.raises(OverflowError):
        DoubleBitMaskNative(1 << double_num_bits)

    with pytest.raises(OverflowError):
        QuadBitMaskNative(1 << quad_num_bits)


def test_bitmask_pickle():
    assert pickle.loads(pickle.dumps(BitMask16(0), -1)).is_zero()

    mask1 = BitMask16(123)
    data = pickle.dumps(mask1, -1)
    mask2 = pickle.loads(data)
    assert mask1 == mask2

    assert pickle.loads(pickle.dumps(DoubleBitMaskNative(0), -1)).is_zero()

    mask1 = DoubleBitMaskNative(0xffff0001)
    data = pickle.dumps(mask1, -1)
    mask2 = pickle.loads(data)
    assert mask1 == mask2

    mask1 = DoubleBitMaskNative(0x7fffffffffffffff)
    data = pickle.dumps(mask1, -1)
    mask2 = pickle.loads(data)
    assert mask1 == mask2

    mask1 = DoubleBitMaskNative(1 << (double_num_bits - 1))
    data = pickle.dumps(mask1, -1)
    mask2 = pickle.loads(data)
    assert mask1 == mask2
