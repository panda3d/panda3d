from panda3d.core import BitArray, SparseArray
import pickle
import pytest


def test_bitarray_type():
    assert BitArray.get_class_type().name == "BitArray"


def test_bitarray_constructor():
    assert BitArray().is_zero()
    assert BitArray(0).is_zero()

    ba = BitArray(0x10000000000000000000000000)
    assert ba.get_lowest_on_bit() == 100
    assert ba.get_highest_on_bit() == 100

    with pytest.raises(Exception):
        assert BitArray(-1)

    with pytest.raises(Exception):
        assert BitArray(-10000000000000000000)


def test_bitarray_constructor_sparse():
    # Create a BitArray from a SparseArray.
    ba = BitArray(SparseArray.all_on())
    assert ba.is_all_on()

    ba = BitArray(SparseArray())
    assert ba.is_zero()

    sa = SparseArray()
    sa.set_range(3, 64)
    sa.set_range(0, 1)
    sa.clear_range(60, 2)
    ba = BitArray(sa)
    exp = 0b1111100111111111111111111111111111111111111111111111111111111111001
    assert ba.__getstate__() == exp

    sa.invert_in_place()
    ba = BitArray(sa)
    assert ba.__getstate__() == ~exp


def test_bitarray_allon():
    assert BitArray.all_on().is_all_on()
    assert BitArray.all_on().get_highest_on_bit() == -1
    assert BitArray.all_on().get_highest_off_bit() == -1


def test_bitarray_nonzero():
    assert not BitArray()
    assert not BitArray(0)
    assert BitArray(1)
    assert BitArray.all_on()


def test_bitarray_invert():
    assert ~BitArray(0) != BitArray(0)
    assert (~BitArray(0)).is_all_on()
    assert ~~BitArray(0) == BitArray(0)
    assert ~~BitArray(123) == BitArray(123)


def test_bitarray_set_word():
    # Non-inverted
    expected = 9876 | (123456 << (BitArray.num_bits_per_word * 3))
    ba = BitArray(0)
    ba.set_word(0, 9876)
    ba.set_word(3, 123456)
    assert ba.__getstate__() == expected
    assert not ba.is_all_on()

    # Inverted
    ba = BitArray(0)
    ba.invert_in_place()
    ba.set_word(2, 1234)
    full_word = (1 << BitArray.num_bits_per_word) - 1
    expected = ~((full_word & ~1234) << (BitArray.num_bits_per_word * 2))
    assert ba.__getstate__() == expected
    assert not ba.is_all_on()


def test_bitarray_clear():
    ba = BitArray(1234)
    ba.clear()
    assert ba.is_zero()
    assert not ba.is_all_on()
    assert ba.get_highest_on_bit() == -1
    assert ba.get_highest_off_bit() == -1

    ba = BitArray.all_on()
    ba.clear()
    assert ba.is_zero()
    assert not ba.is_all_on()
    assert ba.get_highest_on_bit() == -1
    assert ba.get_highest_off_bit() == -1


def test_bitarray_getstate():
    assert BitArray().__getstate__() == 0
    assert BitArray(0).__getstate__() == 0
    assert BitArray(100).__getstate__() == 100
    assert BitArray(9870000000000000000).__getstate__() == 9870000000000000000
    assert BitArray.all_on().__getstate__() == -1
    assert (~BitArray(100).__getstate__()) == ~100
    assert (~BitArray(812000000000000000).__getstate__()) == ~812000000000000000


def test_bitarray_pickle():
    ba = BitArray()
    assert ba == pickle.loads(pickle.dumps(ba, -1))

    ba = BitArray(0)
    assert ba == pickle.loads(pickle.dumps(ba, -1))

    ba = BitArray(123)
    assert ba == pickle.loads(pickle.dumps(ba, -1))

    ba = BitArray(1 << 128)
    assert ba == pickle.loads(pickle.dumps(ba, -1))

    ba = BitArray(94187049178237918273981729127381723)
    assert ba == pickle.loads(pickle.dumps(ba, -1))

    ba = ~BitArray(94187049178237918273981729127381723)
    assert ba == pickle.loads(pickle.dumps(ba, -1))


def test_bitarray_has_any_of():
    ba = BitArray()
    assert not ba.has_any_of(100, 200)

    ba = BitArray()
    ba.set_range(0, 53)
    assert ba.has_any_of(52, 1)
    assert ba.has_any_of(52, 100)
    assert not ba.has_any_of(53, 45)

    ba = BitArray()
    ba.invert_in_place()
    assert ba.has_any_of(0, 1)
    assert ba.has_any_of(53, 45)
    assert ba.has_any_of(0, 100)
