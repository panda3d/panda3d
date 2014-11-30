###############################################################################
# Name: calllock.py                                                           #
# Purpose: Manager to lock the context of a function call.                    #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2010 Cody Precord <staff@editra.org>                         #
# Licence: wxWindows Licence                                                  #
###############################################################################

"""
Editra Business Model Library: CallLock

Provides a Lock class for managing a lock during the duration of a function
call.

Example:

lock = CallLock(DoSomething)
lock.Lock() # Executes DoSomething


"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: calllock.py 65794 2010-10-13 14:10:09Z CJP $"
__revision__ = "$Revision: 65794 $"

__all__ = [ 'CallLock', 'StaticCallLock', 'LockCall']

#-----------------------------------------------------------------------------#

class CallLock(object):
    """Class to lock a context around a function call"""
    def __init__(self, callable=None, args=[], kwargs={}):
        super(CallLock, self).__init__()

        # Attributes
        self._locked = False
        self.funct = callable
        self.args = args
        self.kwargs = kwargs

    def Discard(self):
        """Clear callable"""
        assert not self.IsLocked(), "Failed to obtain lock!"
        self.funct = None
        self.args = []
        self.kwargs = {}

    def IsLocked(self):
        return self._locked

    def Lock(self):
        assert not self.IsLocked(), "Failed to obtain lock!"
        assert callable(self.funct), "No Callable to Lock!"
        self._locked = True
        rval = self.funct(*self.args, **self.kwargs)
        self._locked = False
        return rval

    def SetManagedCall(self, callable, args=[], kwargs={}):
        """Set the call that will be managed by this lock"""
        assert not self.IsLocked(), "Failed to obtain lock!"
        self.funct = callable
        self.args = args
        self.kwargs = kwargs

#-----------------------------------------------------------------------------#

class StaticCallLock(CallLock):
    """Provides a static lock around a function call"""
    _staticlock = False

    def IsLocked(self):
        return StaticCallLock._staticlock

    def Lock(self):
        """Lock the static class member"""
        StaticCallLock._staticlock = True
        super(StaticCallLock, self).Lock()
        StaticCallLock._staticlock = False

#-----------------------------------------------------------------------------#

def LockCall(lock, callable, args=[], kwargs={}):
    """Convenience function for locking an function call with
    the provided CallLock object.

    """
    if not isinstance(lock, CallLock):
        raise TypeError("lock is not of type CallLock")

    lock.SetManagedCall(callable, args, kwargs)
    rval = lock.Lock()
    lock.Discard()
    return rval
