from panda3d.core import Mutex, ReMutex
from panda3d import core
from random import random
import pytest
import sys


def test_mutex_acquire_release():
    m = Mutex()
    m.acquire()

    # Assert that the lock is truly held now
    assert m.debug_is_locked()

    # Release the lock
    m.release()

    # Make sure the lock is properly released
    assert m.try_acquire()

    # Clean up
    m.release()


def test_mutex_try_acquire():
    m = Mutex()

    # Trying to acquire the lock should succeed
    assert m.try_acquire()

    # Assert that the lock is truly held now
    assert m.debug_is_locked()

    # Clean up
    m.release()


def test_mutex_with():
    m = Mutex()

    rc = sys.getrefcount(m)
    with m:
        assert m.debug_is_locked()

    with m:
        assert m.debug_is_locked()

    assert rc == sys.getrefcount(m)


@pytest.mark.skipif(not core.Thread.is_threading_supported(),
                    reason="Threading support disabled")
def test_mutex_contention():
    # As a smoke test for mutexes, we just spawn a bunch of threads that do a
    # lot of mutexing and hope that we can catch any obvious issues with the
    # mutex implementation, especially when compiling with DEBUG_THREADS.
    m1 = Mutex()
    m2 = Mutex()
    m3 = Mutex()
    m4 = Mutex()

    def thread_acq_rel(m):
        for i in range(5000):
            m.acquire()
            m.release()

    def thread_nested():
        for i in range(5000):
            m1.acquire()
            m4.acquire()
            m4.release()
            m1.release()

    def thread_hand_over_hand():
        m1.acquire()
        for i in range(5000):
            m2.acquire()
            m1.release()
            m3.acquire()
            m2.release()
            m1.acquire()
            m3.release()

        m1.release()

    def thread_sleep(m):
        for i in range(250):
            m.acquire()
            core.Thread.sleep(random() * 0.003)
            m.release()

    threads = [
        core.PythonThread(thread_acq_rel, (m1,), "", ""),
        core.PythonThread(thread_acq_rel, (m2,), "", ""),
        core.PythonThread(thread_acq_rel, (m3,), "", ""),
        core.PythonThread(thread_acq_rel, (m4,), "", ""),
        core.PythonThread(thread_nested, (), "", ""),
        core.PythonThread(thread_nested, (), "", ""),
        core.PythonThread(thread_nested, (), "", ""),
        core.PythonThread(thread_hand_over_hand, (), "", ""),
        core.PythonThread(thread_hand_over_hand, (), "", ""),
        core.PythonThread(thread_sleep, (m1,), "", ""),
        core.PythonThread(thread_sleep, (m2,), "", ""),
        core.PythonThread(thread_sleep, (m3,), "", ""),
        core.PythonThread(thread_sleep, (m4,), "", ""),
    ]
    for thread in threads:
        thread.start(core.TP_normal, True)

    for thread in threads:
        thread.join()


def test_remutex_acquire_release():
    m = ReMutex()
    m.acquire()
    m.acquire()
    m.release()
    m.release()


def test_remutex_try_acquire():
    m = ReMutex()

    # Trying to acquire the lock should succeed
    assert m.try_acquire()

    # Should report being locked
    assert m.debug_is_locked()

    # Trying a second time should succeed
    assert m.try_acquire()

    # Should still report being locked
    assert m.debug_is_locked()

    # Clean up
    m.release()
    m.release()


def test_remutex_with():
    m = ReMutex()

    rc = sys.getrefcount(m)
    with m:
        assert m.debug_is_locked()
        with m:
            assert m.debug_is_locked()
        assert m.debug_is_locked()

    assert rc == sys.getrefcount(m)
