from panda3d.core import PythonTask
from contextlib import contextmanager
import pytest
import types
import sys
import gc


@contextmanager
def gc_disabled():
    gc.disable()
    gc.collect()
    gc.freeze()
    gc.set_debug(gc.DEBUG_SAVEALL)

    try:
        yield
    finally:
        gc.set_debug(0)
        gc.garbage.clear()
        gc.unfreeze()
        gc.collect()
        gc.enable()


def test_pythontask_property_builtin():
    task = PythonTask()

    # Read-write property
    assert task.name == ""
    task.name = "ABC"

    # Read-only property
    assert task.dt == 0.0
    with pytest.raises(AttributeError):
        task.dt = 1.0
    assert task.dt == 0.0

    # Non-existent property
    with pytest.raises(AttributeError):
        task.abc


def test_pythontask_property_custom():
    task = PythonTask()
    assert not hasattr(task, 'custom_field')

    task.custom_field = 1.0
    assert hasattr(task, 'custom_field')
    assert task.custom_field == 1.0

    task.custom_field = 2.0
    assert task.custom_field == 2.0

    del task.custom_field
    assert not hasattr(task, 'custom_field')


def test_pythontask_property_override():
    task = PythonTask()
    assert isinstance(task.gather, types.BuiltinMethodType)

    task.gather = 123
    assert task.gather == 123

    del task.gather
    assert isinstance(task.gather, types.BuiltinMethodType)


def test_pythontask_dict_get():
    task = PythonTask()

    d = task.__dict__
    rc1 = sys.getrefcount(d)

    task.__dict__
    task.__dict__

    rc2 = sys.getrefcount(d)

    assert rc1 == rc2


def test_pythontask_dict_set():
    task = PythonTask()
    d = {}

    rc1 = sys.getrefcount(d)
    task.__dict__ = d
    rc2 = sys.getrefcount(d)

    assert rc1 + 1 == rc2

    task.__dict__ = {}
    rc2 = sys.getrefcount(d)

    assert rc1 == rc2


def test_pythontask_cycle():
    with gc_disabled():
        task = PythonTask()
        assert gc.is_tracked(task)
        task.marker = 'test_pythontask_cycle'
        task.prop = task

        del task

        gc.collect()
        assert len(gc.garbage) > 0

        for g in gc.garbage:
            if isinstance(g, PythonTask) and \
               getattr(g, 'marker', None) == 'test_pythontask_cycle':
                break
        else:
            pytest.fail('not found in garbage')
