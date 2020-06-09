from panda3d.core import BitArray


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
