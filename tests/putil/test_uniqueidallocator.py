from panda3d.core import UniqueIdAllocator


# Value from panda/src/putil/uniqueIdAllocator.cxx, not published
IndexEnd = 0xFFFFFFFF


def test_inclusive_allocation():
    allocator = UniqueIdAllocator(0, 0)
    assert allocator.allocate() == 0
    assert allocator.allocate() == IndexEnd


def test_normal_allocation():
    allocator = UniqueIdAllocator(0, 10)

    for i in range(10 + 1):
        assert allocator.allocate() == i

    assert allocator.allocate() == IndexEnd


def test_min_value_allocation():
    allocator = UniqueIdAllocator(1, 5)

    for i in range(1, 5 + 1):
        assert allocator.allocate() == i

    assert allocator.allocate() == IndexEnd


def test_regular_is_allocated():
    allocator = UniqueIdAllocator(1, 5)

    for i in range(1, 5 + 1):
        assert not allocator.is_allocated(i)

    for i in range(1, 5 + 1):
        assert allocator.is_allocated(allocator.allocate())


def test_bounded_is_allocated():
    allocator = UniqueIdAllocator(1, 5)

    for i in range(1, 5 + 1):
        assert allocator.is_allocated(allocator.allocate())

    assert not allocator.is_allocated(0) # Out of range, left side
    assert not allocator.is_allocated(10) # Out of range, right side


def test_initial_reserve_id():
    allocator = UniqueIdAllocator(1, 3)

    assert not allocator.is_allocated(2)
    allocator.initial_reserve_id(2)
    assert allocator.is_allocated(2)

    assert allocator.allocate() == 1
    assert allocator.allocate() == 3
    assert allocator.allocate() == IndexEnd


def test_initial_reserve_id_exhaustion():
    allocator = UniqueIdAllocator(1, 3)

    for i in range(1, 3 + 1):
        allocator.initial_reserve_id(i)

    assert allocator.allocate() == IndexEnd


def test_free():
    allocator = UniqueIdAllocator(0, 0)

    assert allocator.allocate() == 0
    assert allocator.is_allocated(0)
    assert allocator.free(0)
    assert not allocator.is_allocated(0)


def test_free_reallocation():
    allocator = UniqueIdAllocator(1, 5)

    for i in range(1, 5 + 1):
        assert allocator.allocate() == i
        assert allocator.is_allocated(i)

    for i in range(1, 5 + 1):
        assert allocator.free(i)

    for i in range(1, 5 + 1):
        assert not allocator.is_allocated(i)
        assert allocator.allocate() == i

    assert allocator.allocate() == IndexEnd


def test_free_unallocated():
    allocator = UniqueIdAllocator(0, 2)

    assert allocator.allocate() == 0
    assert allocator.free(0)

    for i in range(0, 2 + 1):
        assert not allocator.free(i)


def test_free_bounds():
    allocator = UniqueIdAllocator(1, 3)

    assert not allocator.free(0) # Out of range, left side
    assert not allocator.free(4) # Out of range, right side


def test_free_reallocation_mid():
    allocator = UniqueIdAllocator(1, 5)

    for i in range(1, 5 + 1):
        assert allocator.allocate() == i
        assert allocator.is_allocated(i)

    assert allocator.free(2)
    assert allocator.free(3)

    assert allocator.allocate() == 2
    assert allocator.allocate() == 3
    assert allocator.allocate() == IndexEnd


def test_free_initial_reserve_id():
    allocator = UniqueIdAllocator(1, 3)

    allocator.initial_reserve_id(1)
    assert allocator.free(1)
    assert allocator.allocate() == 2
    assert allocator.allocate() == 3
    assert allocator.allocate() == 1
    assert allocator.allocate() == IndexEnd


def test_fraction_used():
    allocator = UniqueIdAllocator(1, 4)

    assert allocator.fraction_used() == 0

    for fraction in (0.25, 0.5, 0.75, 1):
        allocator.allocate()
        assert allocator.fraction_used() == fraction

    assert allocator.allocate() == IndexEnd
