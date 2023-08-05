""" This module reimplements Python's native threading module using Panda
threading constructs.  It's designed as a drop-in replacement for the
threading module for code that works with Panda; it is necessary because
in some compilation models, Panda's threading constructs are
incompatible with the OS-provided threads used by Python's thread
module.

This module implements the threading module with a thin layer over
Panda's threading constructs.  As such, the semantics are close to,
but not precisely, the semantics documented for Python's standard
threading module.  If you really do require strict adherence to
Python's semantics, see the threading2 module instead.

However, if you don't need such strict adherence to Python's original
semantics, this module is probably a better choice.  It is likely to
be slighly faster than the threading2 module (and even slightly faster
than Python's own threading module).  It is also better integrated
with Panda's threads, so that Panda's thread debug mechanisms will be
easier to use and understand.

It is permissible to mix-and-match both threading and threading2
within the same application. """

from panda3d import core
from direct.stdpy import thread as _thread
import sys as _sys

import weakref

__all__ = [
    'Thread',
    'Lock', 'RLock',
    'Condition',
    'Semaphore', 'BoundedSemaphore',
    'Event',
    'Timer',
    'ThreadError',
    'local',
    'current_thread',
    'main_thread',
    'enumerate', 'active_count',
    'settrace', 'setprofile', 'stack_size',
    'TIMEOUT_MAX',
]

TIMEOUT_MAX = _thread.TIMEOUT_MAX

local = _thread._local
_newname = _thread._newname
ThreadError = _thread.error


class ThreadBase:
    """ A base class for both Thread and ExternalThread in this
    module. """

    def __init__(self):
        pass

    def getName(self):
        return self.name

    def isDaemon(self):
        return self.daemon

    def setDaemon(self, daemon):
        if self.is_alive():
            raise RuntimeError

        self.__dict__['daemon'] = daemon

    def __setattr__(self, key, value):
        if key == 'name':
            self.setName(value)
        elif key == 'ident':
            raise AttributeError
        elif key == 'daemon':
            self.setDaemon(value)
        else:
            self.__dict__[key] = value


# Copy these static methods from Panda's Thread object.  These are
# useful if you may be running in Panda's SIMPLE_THREADS compilation
# mode.
ThreadBase.forceYield = core.Thread.forceYield  # type: ignore[attr-defined]
ThreadBase.considerYield = core.Thread.considerYield  # type: ignore[attr-defined]


class Thread(ThreadBase):
    """ This class provides a wrapper around Panda's PythonThread
    object.  The wrapper is designed to emulate Python's own
    threading.Thread object. """

    def __init__(self, group=None, target=None, name=None, args=(), kwargs={}, daemon=None):
        ThreadBase.__init__(self)

        assert group is None
        self.__target = target
        self.__args = args
        self.__kwargs = kwargs

        if not name:
            name = _newname()

        current = current_thread()
        if daemon is not None:
            self.__dict__['daemon'] = daemon
        else:
            self.__dict__['daemon'] = current.daemon
        self.__dict__['name'] = name

        def call_run():
            # As soon as the thread is done, break the circular reference.
            try:
                self.run()
            finally:
                self.__thread = None
                _thread._remove_thread_id(self.ident)

        self.__thread = core.PythonThread(call_run, None, name, name)
        threadId = _thread._add_thread(self.__thread, weakref.proxy(self))
        self.__dict__['ident'] = threadId

    def __del__(self):
        _thread._remove_thread_id(self.ident)

    def is_alive(self):
        thread = self.__thread
        return thread is not None and thread.is_started()

    isAlive = is_alive

    def start(self):
        thread = self.__thread
        if thread is None or thread.is_started():
            raise RuntimeError

        if not thread.start(core.TPNormal, True):
            raise RuntimeError

    def run(self):
        if _settrace_func:
            _sys.settrace(_settrace_func)
        if _setprofile_func:
            _sys.setprofile(_setprofile_func)

        self.__target(*self.__args, **self.__kwargs)

    def join(self, timeout = None):
        # We don't support a timed join here, sorry.
        assert timeout is None
        thread = self.__thread
        if thread is not None:
            thread.join()
            # Clear the circular reference.
            self.__thread = None
            _thread._remove_thread_id(self.ident)

    def setName(self, name):
        self.__dict__['name'] = name
        self.__thread.setName(name)


class ExternalThread(ThreadBase):
    """ Returned for a Thread object that wasn't created by this
    interface. """

    def __init__(self, extThread, threadId):
        ThreadBase.__init__(self)

        self.__thread = extThread
        self.__dict__['daemon'] = True
        self.__dict__['name'] = self.__thread.getName()
        self.__dict__['ident'] = threadId

    def is_alive(self):
        return self.__thread.isStarted()

    def isAlive(self):
        return self.__thread.isStarted()

    def start(self):
        raise RuntimeError

    def run(self):
        raise RuntimeError

    def join(self, timeout = None):
        raise RuntimeError

    def setDaemon(self, daemon):
        raise RuntimeError


class MainThread(ExternalThread):
    """ Returned for the MainThread object. """

    def __init__(self, extThread, threadId):
        ExternalThread.__init__(self, extThread, threadId)
        self.__dict__['daemon'] = False


class Lock(core.Mutex):
    """ This class provides a wrapper around Panda's Mutex object.
    The wrapper is designed to emulate Python's own threading.Lock
    object. """

    def __init__(self, name = "PythonLock"):
        core.Mutex.__init__(self, name)


class RLock(core.ReMutex):
    """ This class provides a wrapper around Panda's ReMutex object.
    The wrapper is designed to emulate Python's own threading.RLock
    object. """

    def __init__(self, name = "PythonRLock"):
        core.ReMutex.__init__(self, name)


class Condition(core.ConditionVar):
    """ This class provides a wrapper around Panda's ConditionVar
    object.  The wrapper is designed to emulate Python's own
    threading.Condition object. """

    def __init__(self, lock = None):
        if not lock:
            lock = Lock()

        # Panda doesn't support RLock objects used with condition
        # variables.
        assert isinstance(lock, Lock)

        self.__lock = lock
        core.ConditionVar.__init__(self, self.__lock)

    def acquire(self, *args, **kw):
        return self.__lock.acquire(*args, **kw)

    def release(self):
        self.__lock.release()

    def wait(self, timeout = None):
        if timeout is None:
            core.ConditionVar.wait(self)
        else:
            core.ConditionVar.wait(self, timeout)

    def notifyAll(self):
        core.ConditionVar.notifyAll(self)

    notify_all = notifyAll

    __enter__ = acquire

    def __exit__(self, t, v, tb):
        self.release()


class Semaphore(core.Semaphore):
    """ This class provides a wrapper around Panda's Semaphore
    object.  The wrapper is designed to emulate Python's own
    threading.Semaphore object. """

    def __init__(self, value = 1):
        core.Semaphore.__init__(self, value)

    def acquire(self, blocking = True):
        if blocking:
            core.Semaphore.acquire(self)
            return True
        else:
            return core.Semaphore.tryAcquire(self)

    __enter__ = acquire

    def __exit__(self, t, v, tb):
        self.release()


class BoundedSemaphore(Semaphore):
    """ This class provides a wrapper around Panda's Semaphore
    object.  The wrapper is designed to emulate Python's own
    threading.BoundedSemaphore object. """

    def __init__(self, value = 1):
        self.__max = value
        Semaphore.__init__(value)

    def release(self):
        if self.getCount() > self.__max:
            raise ValueError

        Semaphore.release(self)


class Event:
    """ This class is designed to emulate Python's own threading.Event
    object. """

    def __init__(self):
        self.__lock = core.Mutex("Python Event")
        self.__cvar = core.ConditionVar(self.__lock)
        self.__flag = False

    def is_set(self):
        return self.__flag

    isSet = is_set

    def set(self):
        self.__lock.acquire()
        try:
            self.__flag = True
            self.__cvar.notifyAll()

        finally:
            self.__lock.release()

    def clear(self):
        self.__lock.acquire()
        try:
            self.__flag = False

        finally:
            self.__lock.release()

    def wait(self, timeout = None):
        self.__lock.acquire()
        try:
            if timeout is None:
                while not self.__flag:
                    self.__cvar.wait()
            else:
                clock = core.TrueClock.getGlobalPtr()
                expires = clock.getShortTime() + timeout
                while not self.__flag:
                    wait = expires - clock.getShortTime()
                    if wait < 0:
                        return

                    self.__cvar.wait(wait)

        finally:
            self.__lock.release()


class Timer(Thread):
    """Call a function after a specified number of seconds:

    t = Timer(30.0, f, args=[], kwargs={})
    t.start()
    t.cancel() # stop the timer's action if it's still waiting
    """

    def __init__(self, interval, function, args=[], kwargs={}):
        Thread.__init__(self)
        self.interval = interval
        self.function = function
        self.args = args
        self.kwargs = kwargs
        self.finished = Event()

    def cancel(self):
        """Stop the timer if it hasn't finished yet"""
        self.finished.set()

    def run(self):
        self.finished.wait(self.interval)
        if not self.finished.isSet():
            self.function(*self.args, **self.kwargs)
        self.finished.set()


def _create_thread_wrapper(t, threadId):
    """ Creates a thread wrapper for the indicated external thread. """
    if isinstance(t, core.MainThread):
        pyt = MainThread(t, threadId)
    else:
        pyt = ExternalThread(t, threadId)

    return pyt


def current_thread():
    t = core.Thread.getCurrentThread()
    return _thread._get_thread_wrapper(t, _create_thread_wrapper)


def main_thread():
    t = core.Thread.getMainThread()
    return _thread._get_thread_wrapper(t, _create_thread_wrapper)


currentThread = current_thread


def enumerate():
    tlist = []
    _thread._threadsLock.acquire()
    try:
        for thread, locals, wrapper in list(_thread._threads.values()):
            if wrapper and wrapper.is_alive():
                tlist.append(wrapper)
        return tlist
    finally:
        _thread._threadsLock.release()


def active_count():
    return len(enumerate())


activeCount = active_count

_settrace_func = None


def settrace(func):
    global _settrace_func
    _settrace_func = func


_setprofile_func = None


def setprofile(func):
    global _setprofile_func
    _setprofile_func = func


def stack_size(size = None):
    raise ThreadError
