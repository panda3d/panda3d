from panda3d.core import Event, EventQueue
from contextlib import contextmanager
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


def test_event_subclass_gc():
    class PythonEvent(Event):
        pass

    # Only subclasses should be tracked by the GC
    assert not gc.is_tracked(Event('test'))

    # Python wrapper destructs last
    with gc_disabled():
        event = PythonEvent('test_event_subclass_gc1')

        assert event in gc.get_objects()

        event = None

        gc.collect()
        assert len(gc.garbage) == 1
        assert gc.garbage[0].name == 'test_event_subclass_gc1'

    # C++ reference destructs last
    with gc_disabled():
        event = PythonEvent('test_event_subclass_gc2')
        queue = EventQueue()
        queue.queue_event(event)

        assert event in gc.get_objects()

        event = None

        # Hasn't been collected yet, since parent still holds a reference
        gc.collect()
        assert len(gc.garbage) == 0
        assert 'test_event_subclass_gc2' in [obj.name for obj in gc.get_objects() if isinstance(obj, Event)]

        queue = None

        # Destructed without needing the GC
        assert 'test_event_subclass_gc2' not in [obj.name for obj in gc.get_objects() if isinstance(obj, Event)]

        gc.collect()
        assert len(gc.garbage) == 0
