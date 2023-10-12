from panda3d.core import BitArray
import pickle
import pytest


def test_bitarray_allon():
    assert BitArray.all_on().is_all_on()


def test_bitarray_invert():
    assert ~BitArray(0) != BitArray(0)
    assert (~BitArray(0)).is_all_on()
    assert ~~BitArray(0) == BitArray(0)
    assert ~~BitArray(123) == BitArray(123)


def test_bitarray_getstate():
    assert BitArray().__getstate__() == 0
    assert BitArray(0).__getstate__() == 0
    assert BitArray(100).__getstate__() == 100
    assert BitArray.all_on().__getstate__() == -1
    assert ~BitArray(100).__getstate__() == ~100


def test_bitarray_pickle():
    ba = BitArray()
    assert ba == pickle.loads(pickle.dumps(ba, -1))

    ba = BitArray(0)
    assert ba == pickle.loads(pickle.dumps(ba, -1))

    ba = BitArray(123)
    assert ba == pickle.loads(pickle.dumps(ba, -1))

    ba = BitArray(1 << 128)
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
