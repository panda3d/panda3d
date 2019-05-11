from panda3d import core


def test_sparse_array_set_bit_to():
    """Tests SparseArray behavior for set_bit_to()."""

    s = core.SparseArray()
    s.set_bit_to(5, True)
    assert s.get_bit(5)

    s = core.SparseArray.all_on()
    s.set_bit_to(5, False)
    assert not s.get_bit(5)


def test_sparse_array_clear():
    """Tests SparseArray behavior for clear()."""

    s = core.SparseArray.all_on()
    s.clear()
    assert s.is_zero()
    assert not s.is_inverse()
    assert s.get_num_subranges() == 0
    assert s.get_num_on_bits() == 0
    assert s.get_num_bits() == 0

    s = core.SparseArray()
    s.set_range(5, 10)
    s.clear()
    assert s.is_zero()
    assert not s.is_inverse()
    assert s.get_num_subranges() == 0
    assert s.get_num_on_bits() == 0
    assert s.get_num_bits() == 0


def test_sparse_array_clear_range():
    # Not using parametrize because there are too many values for that.
    for mask in range(0x7f):
        for begin in range(8):
            for size in range(8):
                b = core.BitArray(mask)
                s = core.SparseArray(b)

                s.clear_range(begin, size)
                b.clear_range(begin, size)

                assert core.BitArray(s) == b
                assert s == core.SparseArray(b)


def test_sparse_array_set_clear_ranges():
    """Tests SparseArray behavior for setting and clearing ranges."""

    # test clear_range with single overlapping on-range
    # (clear_range extends beyond highest on-bit)
    s = core.SparseArray()
    s.set_range(2, 3)
    s.clear_range(3, 3)
    assert s.get_bit(2)
    assert not s.get_bit(3)

    # same as above, using set_range_to
    s = core.SparseArray()
    s.set_range_to(True, 2, 3)
    s.set_range_to(False, 3, 3)
    assert s.get_bit(2)
    assert not s.get_bit(3)

    # test clear_range using off-range which overlaps two on-ranges
    # (lowest off-bit in lowest on-range, highest off-bit in highest on-range)
    s = core.SparseArray()
    s.set_range(2, 3)
    s.set_range(7, 3)
    s.clear_range(3, 6)
    assert s.get_bit(2)
    assert not s.get_bit(3)
    assert not s.get_bit(8)
    assert s.get_bit(9)

    # same as above, using set_range_to
    s = core.SparseArray()
    s.set_range_to(True, 2, 3)
    s.set_range_to(True, 7, 3)
    s.set_range_to(False, 3, 6)
    assert s.get_bit(2)
    assert not s.get_bit(3)
    assert not s.get_bit(8)
    assert s.get_bit(9)


def test_sparse_array_set_range():
    """Tests SparseArray behavior for set_range()."""

    # test set_range with single overlapping off-range
    # (set_range extends beyond highest off-bit)
    s = core.SparseArray.all_on()
    s.clear_range(2, 3)
    s.set_range(3, 3)
    assert not s.get_bit(2)
    assert s.get_bit(3)

    # same as above, using set_range_to
    s = core.SparseArray.all_on()
    s.set_range_to(False, 2, 3)
    s.set_range_to(True, 3, 3)
    assert not s.get_bit(2)
    assert s.get_bit(3)

    # test set_range using on-range which overlaps two off-ranges
    # (lowest on-bit in lowest off-range, highest on-bit in highest off-range)
    s = core.SparseArray.all_on()
    s.clear_range(2, 3)
    s.clear_range(7, 3)
    s.set_range(3, 6)
    assert not s.get_bit(2)
    assert s.get_bit(3)
    assert s.get_bit(8)
    assert not s.get_bit(9)

    # same as above, using set_range_to
    s = core.SparseArray.all_on()
    s.set_range_to(False, 2, 3)
    s.set_range_to(False, 7, 3)
    s.set_range_to(True, 3, 6)
    assert not s.get_bit(2)
    assert s.get_bit(3)
    assert s.get_bit(8)
    assert not s.get_bit(9)


def test_sparse_array_bits_in_common():
    """Tests SparseArray behavior for has_bits_in_common()."""

    s = core.SparseArray()
    t = core.SparseArray()
    s.set_range(2, 4)
    t.set_range(5, 4)
    assert s.has_bits_in_common(t)

    s = core.SparseArray()
    t = core.SparseArray()
    s.set_range(2, 4)
    t.set_range(6, 4)
    assert not s.has_bits_in_common(t)


def test_sparse_array_operations():
    """Tests SparseArray behavior for various operations."""

    # test bitshift to left
    s = core.SparseArray()
    s.set_bit(2)
    t = s << 2
    assert t.get_bit(4)
    assert not t.get_bit(2)

    # test bitshift to right
    s = core.SparseArray()
    s.set_bit(4)
    t = s >> 2
    assert t.get_bit(2)
    assert not t.get_bit(4)

    # test bitwise AND
    s = core.SparseArray()
    t = core.SparseArray()
    s.set_bit(2)
    s.set_bit(3)
    t.set_bit(1)
    t.set_bit(3)
    u = s & t
    assert not u.get_bit(0)
    assert not u.get_bit(1)
    assert not u.get_bit(2)
    assert u.get_bit(3)

    # test bitwise OR
    s = core.SparseArray()
    t = core.SparseArray()
    s.set_bit(2)
    s.set_bit(3)
    t.set_bit(1)
    t.set_bit(3)
    u = s | t
    assert not u.get_bit(0)
    assert u.get_bit(1)
    assert u.get_bit(2)
    assert u.get_bit(3)

    # test bitwise XOR
    s = core.SparseArray()
    t = core.SparseArray()
    s.set_bit(2)
    s.set_bit(3)
    t.set_bit(1)
    t.set_bit(3)
    u = s ^ t
    assert not u.get_bit(0)
    assert u.get_bit(1)
    assert u.get_bit(2)
    assert not u.get_bit(3)


def test_sparse_array_augm_assignment():
    """Tests SparseArray behavior for augmented assignments."""

    # test in-place bitshift to left
    s = t = core.SparseArray()
    t <<= 2
    assert s is t

    # test in-place bitshift to right
    s = t = core.SparseArray()
    t >>= 2
    assert s is t

    # test in-place bitwise AND
    s = t = core.SparseArray()
    u = core.SparseArray()
    t &= u
    assert s is t

    # test in-place bitwise OR
    s = t = core.SparseArray()
    u = core.SparseArray()
    t |= u
    assert s is t

    # test in-place bitwise XOR
    s = t = core.SparseArray()
    u = core.SparseArray()
    t ^= u
    assert s is t
