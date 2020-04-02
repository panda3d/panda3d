from panda3d.core import BitArray
import pickle
import pytest


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


def test_bitarray_allon():
    assert BitArray.all_on().is_all_on()


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

    ba = BitArray(94187049178237918273981729127381723)
    assert ba == pickle.loads(pickle.dumps(ba, -1))

    ba = ~BitArray(94187049178237918273981729127381723)
    assert ba == pickle.loads(pickle.dumps(ba, -1))
