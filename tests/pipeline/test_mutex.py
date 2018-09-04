from panda3d.core import Mutex, ReMutex


def test_mutex_acquire_release():
    m = Mutex()
    m.acquire()

    # Assert that the lock is truly held now
    assert not m.try_acquire()

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
    assert not m.try_acquire()

    # Clean up
    m.release()


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

    # Trying a second time should succeed
    assert m.try_acquire()

    # Clean up
    m.release()
    m.release()

