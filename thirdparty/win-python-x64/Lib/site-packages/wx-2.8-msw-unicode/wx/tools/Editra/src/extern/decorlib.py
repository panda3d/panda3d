"""
Collection of helpful decorator methods

"""

__all__ = ['anythread',]

#-----------------------------------------------------------------------------#
# Imports
import wx
import threading

#-----------------------------------------------------------------------------#

#
#  <Found on PyPi: License: Public Domain>
#  Author: Ryan Kelly
#
#  wxAnyThread:  allow methods on wxPython objects to be called from any thread
#
# In wxPython, methods that alter the state of the GUI are only safe to call from
# the thread running the main event loop.  Other threads must typically post
# events to the GUI thread instead of invoking methods directly.
#
# While there are builtin shortcuts for this (e.g. wx.CallAfter) they do not
# capture the full semantics of a function call.  This module provides an easy
# way to invoke methods from any thread *transparently*, propagating return
# values and exceptions back to the calling thread.
#
# The main interface is a decorator named "anythread", which can be applied
# to methods to make them safe to call from any thread, like so:
#
#  class MyFrame(wx.Frame):
#
#     @anythread
#     def ShowFancyStuff():
#         dlg = MyQueryDialog(self,"Enter some data")
#         if dlg.ShowModal() == wx.ID_OK:
#             resp = dlg.GetResponse()
#             return int(resp)
#         else:
#             raise NoDataEnteredError()
#
# The ShowFancyStuff method can now be directly invoked from any thread.
# The calling thread will block while the main GUI thread shows the dialog,
# and will then receive a return value or exception as appropriate.
#

_EVT_INVOKE_METHOD = wx.NewEventType()

class MethodInvocationEvent(wx.PyEvent):
    """Event fired to the GUI thread indicating a method invocation."""
    def __init__(self, func, args, kwds):
      wx.PyEvent.__init__(self)
      self.SetEventType(_EVT_INVOKE_METHOD)
      self.func = func
      self.args = args
      self.kwds = kwds
      # The calling thread will block on this semaphore, which the GUI
      # thread will release when the results are available.
      # TODO: how expensive are these to create?  Should we re-use them?
      self.blocker = threading.Semaphore(0)

    def invoke(self):
        wx.PostEvent(self.args[0], self)
        self.blocker.acquire()
        try:
            return self.result
        except AttributeError:
            raise self.exception

    def process(self):
        try:
            self.result = self.func(*self.args, **self.kwds)
        except Exception, e:
            self.exception = e
        self.blocker.release()

def handler(evt):
    evt.process()

def anythread(func):
    """Method decorator allowing call from any thread.
    The method is replaced by one that posts a MethodInvocationEvent to the
    object, then blocks waiting for it to be completed.  The target object
    if automatically connected to the _EVT_INVOKE_METHOD event if it wasn't
    alread connected.

    """
    def invoker(*args, **kwds):
      if wx.Thread_IsMain():
        return func(*args, **kwds)
      else:
        self = args[0]
        if not hasattr(self, "_AnyThread__connected"):
          self.Connect(-1, -1, _EVT_INVOKE_METHOD,handler)
          self._AnyThread__connected = True
        evt = MethodInvocationEvent(func, args, kwds)
        return evt.invoke()

    invoker.__name__ = func.__name__
    invoker.__doc__ = func.__doc__
    return invoker

#-----------------------------------------------------------------------------#
