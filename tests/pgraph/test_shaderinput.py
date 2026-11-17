from panda3d.core import ShaderInput, Vec4
from array import array


def test_shaderinput_construct_sequence_int():
    i = ShaderInput('test', array('I', [1, 2, 3, 4]))


def test_shaderinput_vector_compare():
    i0 = ShaderInput('a', Vec4(0, 0, 0, 0))
    i1 = ShaderInput('a', Vec4(1e-9, 0, 0, 0))
    i2 = ShaderInput('a', Vec4(1e-8, 0, 0, 0))
    i3 = ShaderInput('a', Vec4(2, 0, 0, 0))

    assert i0 == i0
    assert i1 == i1
    assert i2 == i2
    assert i3 == i3

    assert i0 != i1
    assert i0 != i2
    assert i0 != i3
    assert i1 != i2
    assert i2 != i3
    assert i1 != i3

    assert not i0 < i0
    assert not i1 < i1
    assert not i2 < i2
    assert not i3 < i3

    assert i0 < i1
    assert i0 < i2
    assert i0 < i3
    assert i1 < i2
    assert i2 < i3
    assert i1 < i3


def test_shaderinput_nested_sequence_exception():
    import pytest
    class ChangingSequence:
        def __len__(self):
            return 4
        def __getitem__(self, index):
            if index == 2:
                raise RuntimeError("nested shader sequence failed")
            if index >= 4:
                raise IndexError(index)
            return 1
    with pytest.raises(RuntimeError, match="nested shader sequence failed"):
        ShaderInput("test", [ChangingSequence()])


def test_shaderinput_nested_sequence_snapshot():
    import threading
    values = [1, 2, 3, 4]
    stop = threading.Event()
    def change():
        while not stop.is_set():
            values.clear()
            values.extend([1, 2, 3, 4])
    thread = threading.Thread(target=change)
    thread.start()
    try:
        for _ in range(4000):
            try:
                ShaderInput("test", [values])
            except TypeError:
                # An empty snapshot has no supported numeric vector shape.
                pass
    finally:
        stop.set()
        thread.join(5)
    assert not thread.is_alive()
