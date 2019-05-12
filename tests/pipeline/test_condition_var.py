from panda3d.core import Mutex, ConditionVarFull
from panda3d import core
from direct.stdpy import thread
import pytest


def yield_thread():
    # Thread.force_yield() is not enough for true-threading builds, whereas
    # time.sleep() does not yield in simple-thread builds.  Thread.sleep()
    # seems to do the job in all cases, however.
    core.Thread.sleep(0.002)


def test_cvar_notify():
    # Just tests that notifying without waiting does no harm.
    m = Mutex()
    cv = ConditionVarFull(m)

    cv.notify()
    cv.notify_all()
    del cv


def test_cvar_notify_locked():
    # Tests the same thing, but with the lock held.
    m = Mutex()
    cv = ConditionVarFull(m)

    with m:
        cv.notify()

    with m:
        cv.notify_all()

    del cv


@pytest.mark.parametrize("num_threads", [1, 2, 3, 4])
@pytest.mark.skipif(not core.Thread.is_threading_supported(),
                    reason="Threading support disabled")
def test_cvar_notify_thread(num_threads):
    # Tests notify() with some number of threads waiting.
    m = Mutex()
    cv = ConditionVarFull(m)

    # We prematurely notify, so that we can test that it's not doing anything.
    m.acquire()
    cv.notify()

    state = {'waiting': 0}

    def wait_thread():
        m.acquire()
        state['waiting'] += 1
        cv.wait()
        state['waiting'] -= 1
        m.release()

    # Start the threads, and yield to it, giving it a chance to mess up.
    threads = []
    for i in range(num_threads):
        thread = core.PythonThread(wait_thread, (), "", "")
        thread.start(core.TP_high, True)

    # Yield until all of the threads are waiting for the condition variable.
    for i in range(1000):
        m.release()
        yield_thread()
        m.acquire()
        if state['waiting'] == num_threads:
            break

    assert state['waiting'] == num_threads

    # OK, now signal it, and yield.  One thread must be unblocked per notify.
    for i in range(num_threads):
        cv.notify()
        expected_waiters = num_threads - i - 1

        for j in range(1000):
            m.release()
            yield_thread()
            m.acquire()
            if state['waiting'] == expected_waiters:
                break

        assert state['waiting'] == expected_waiters

    m.release()
    for thread in threads:
        thread.join()
    cv = None


@pytest.mark.parametrize("num_threads", [1, 2, 3, 4])
@pytest.mark.skipif(not core.Thread.is_threading_supported(),
                    reason="Threading support disabled")
def test_cvar_notify_all_threads(num_threads):
    # Tests notify_all() with some number of threads waiting.
    m = Mutex()
    cv = ConditionVarFull(m)

    # We prematurely notify, so that we can test that it's not doing anything.
    m.acquire()
    cv.notify_all()

    state = {'waiting': 0}

    def wait_thread():
        m.acquire()
        state['waiting'] += 1
        cv.wait()
        state['waiting'] -= 1
        m.release()

    # Start the threads, and yield to it, giving it a chance to mess up.
    threads = []
    for i in range(num_threads):
        thread = core.PythonThread(wait_thread, (), "", "")
        thread.start(core.TP_high, True)

    # Yield until all of the threads are waiting for the condition variable.
    for i in range(1000):
        m.release()
        yield_thread()
        m.acquire()
        if state['waiting'] == num_threads:
            break

    assert state['waiting'] == num_threads

    # OK, now signal it, and yield.  All threads must unblock.
    cv.notify_all()
    for i in range(1000):
        m.release()
        yield_thread()
        m.acquire()
        if state['waiting'] == 0:
            break

    assert state['waiting'] == 0
    m.release()

    for thread in threads:
        thread.join()
    cv = None
