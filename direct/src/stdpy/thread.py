""" This module reimplements Python's native thread module using Panda
threading constructs.  It's designed as a drop-in replacement for the
thread module for code that works with Panda; it is necessary because
in some compilation models, Panda's threading constructs are
incompatible with the OS-provided threads used by Python's thread
module. """

__all__ = [
    'error', 'LockType',
    'start_new_thread',
    'interrupt_main',
    'exit', 'allocate_lock', 'get_ident',
    'stack_size',
    'force_yield', 'consider_yield',
    'forceYield', 'considerYield',
    'TIMEOUT_MAX'
    ]

from panda3d import core
import sys

if sys.platform == "win32":
    TIMEOUT_MAX = float(0xffffffff // 1000)
else:
    TIMEOUT_MAX = float(0x7fffffffffffffff // 1000000000)

# These methods are defined in Panda, and are particularly useful if
# you may be running in Panda's SIMPLE_THREADS compilation mode.
force_yield = core.Thread.force_yield
consider_yield = core.Thread.consider_yield

forceYield = force_yield
considerYield = consider_yield

if sys.version_info >= (3, 3):
    error = RuntimeError
else:
    class error(Exception):
        pass

class LockType:
    """ Implements a mutex lock.  Instead of directly subclassing
    PandaModules.Mutex, we reimplement the lock here, to allow us to
    provide the described Python lock semantics.  In particular, this
    allows a different thread to release the lock than the one that
    acquired it. """

    def __init__(self):
        self.__lock = core.Mutex('PythonLock')
        self.__cvar = core.ConditionVar(self.__lock)
        self.__locked = False

    def acquire(self, waitflag = 1, timeout = -1):
        self.__lock.acquire()
        try:
            if self.__locked and not waitflag:
                return False

            if timeout >= 0:
                while self.__locked:
                    self.__cvar.wait(timeout)
            else:
                while self.__locked:
                    self.__cvar.wait()

            self.__locked = True
            return True

        finally:
            self.__lock.release()

    def release(self):
        self.__lock.acquire()
        try:
            if not self.__locked:
                raise error('Releasing unheld lock.')

            self.__locked = False
            self.__cvar.notify()

        finally:
            self.__lock.release()

    def locked(self):
        return self.__locked

    __enter__ = acquire

    def __exit__(self, t, v, tb):
        self.release()

# Helper to generate new thread names
_counter = 0
def _newname(template="Thread-%d"):
    global _counter
    _counter = _counter + 1
    return template % _counter

_threads = {}
_nextThreadId = 0
_threadsLock = core.Mutex('thread._threadsLock')

def start_new_thread(function, args, kwargs = {}, name = None):
    def threadFunc(threadId, function = function, args = args, kwargs = kwargs):
        try:
            try:
                function(*args, **kwargs)
            except SystemExit:
                pass

        finally:
            _remove_thread_id(threadId)

    global _nextThreadId
    _threadsLock.acquire()
    try:
        threadId = _nextThreadId
        _nextThreadId += 1

        if name is None:
            name = 'PythonThread-%s' % (threadId)

        thread = core.PythonThread(threadFunc, [threadId], name, name)
        thread.setPythonIndex(threadId)
        _threads[threadId] = (thread, {}, None)

        thread.start(core.TPNormal, False)
        return threadId

    finally:
        _threadsLock.release()

def _add_thread(thread, wrapper):
    """ Adds the indicated core.Thread object, with the indicated Python
    wrapper, to the thread list.  Returns the new thread ID. """

    global _nextThreadId
    _threadsLock.acquire()
    try:
        threadId = _nextThreadId
        _nextThreadId += 1

        thread.setPythonIndex(threadId)
        _threads[threadId] = (thread, {}, wrapper)
        return threadId

    finally:
        _threadsLock.release()

def _get_thread_wrapper(thread, wrapperClass):
    """ Returns the thread wrapper for the indicated thread.  If there
    is not one, creates an instance of the indicated wrapperClass
    instead. """

    threadId = thread.getPythonIndex()
    if threadId == -1:
        # The thread has never been assigned a threadId.  Go assign one.

        global _nextThreadId
        _threadsLock.acquire()
        try:
            threadId = _nextThreadId
            _nextThreadId += 1

            thread.setPythonIndex(threadId)
            wrapper = wrapperClass(thread, threadId)
            _threads[threadId] = (thread, {}, wrapper)
            return wrapper

        finally:
            _threadsLock.release()

    else:
        # The thread has been assigned a threadId.  Look for the wrapper.
        _threadsLock.acquire()
        try:
            t, locals, wrapper = _threads[threadId]
            assert t == thread
            if wrapper is None:
                wrapper = wrapperClass(thread, threadId)
                _threads[threadId] = (thread, locals, wrapper)
            return wrapper

        finally:
            _threadsLock.release()

def _get_thread_locals(thread, i):
    """ Returns the locals dictionary for the indicated thread.  If
    there is not one, creates an empty dictionary. """

    threadId = thread.getPythonIndex()
    if threadId == -1:
        # The thread has never been assigned a threadId.  Go assign one.

        global _nextThreadId
        _threadsLock.acquire()
        try:
            threadId = _nextThreadId
            _nextThreadId += 1

            thread.setPythonIndex(threadId)
            locals = {}
            _threads[threadId] = (thread, locals, None)
            return locals.setdefault(i, {})

        finally:
            _threadsLock.release()

    else:
        # The thread has been assigned a threadId.  Get the locals.
        _threadsLock.acquire()
        try:
            t, locals, wrapper = _threads[threadId]
            assert t == thread
            return locals.setdefault(i, {})

        finally:
            _threadsLock.release()


def _remove_thread_id(threadId):
    """ Removes the thread with the indicated ID from the thread list. """

    # On interpreter shutdown, Python may set module globals to None.
    if _threadsLock is None or _threads is None:
        return

    _threadsLock.acquire()
    try:
        if threadId in _threads:
            thread, locals, wrapper = _threads[threadId]
            assert thread.getPythonIndex() == threadId
            del _threads[threadId]
            thread.setPythonIndex(-1)

    finally:
        _threadsLock.release()


def interrupt_main():
    # TODO.
    pass

def exit():
    raise SystemExit

def allocate_lock():
    return LockType()

def get_ident():
    return core.Thread.getCurrentThread().this

def stack_size(size = 0):
    raise error


class _local(object):
    """ This class provides local thread storage using Panda's
    threading system. """

    def __del__(self):
        i = id(self)

        # Delete this key from all threads.
        _threadsLock.acquire()
        try:
            for thread, locals, wrapper in list(_threads.values()):
                try:
                    del locals[i]
                except KeyError:
                    pass

        finally:
            _threadsLock.release()

    def __setattr__(self, key, value):
        d = _get_thread_locals(core.Thread.getCurrentThread(), id(self))
        d[key] = value

    def __getattribute__(self, key):
        d = _get_thread_locals(core.Thread.getCurrentThread(), id(self))
        if key == '__dict__':
            return d
        try:
            return d[key]
        except KeyError:
            return object.__getattribute__(self, key)
