###############################################################################
# Name: osutil.py                                                             #
# Purpose: Text Utilities.                                                    #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2010 Cody Precord <staff@editra.org>                         #
# Licence: wxWindows Licence                                                  #
###############################################################################

"""
Editra Business Model Library: Operating System Utilities

Utilities for handling OS related interactions.

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: $"
__revision__ = "$Revision: $"

__all__ = ['InstallTermHandler', ]

#-----------------------------------------------------------------------------#
# Imports
import wx
import signal
import collections

HASWIN32 = False
if wx.Platform == '__WXMSW__':
    try:
        import win32api
    except ImportError:
        HASWIN32 = False
    else:
        HASWIN32 = True

#-----------------------------------------------------------------------------#

def InstallTermHandler(callback, *args, **kwargs):
    """Install exit app handler for sigterm (unix/linux)
    and uses SetConsoleCtrlHandler on Windows.
    @param callback: callable(*args, **kwargs)
    @return: bool (installed or not)

    """
    assert isinstance(callback, collections.Callable), "callback must be callable!"

    installed = True
    if wx.Platform == '__WXMSW__':
        if HASWIN32:
            win32api.SetConsoleCtrlHandler(lambda dummy : callback(*args, **kwargs),
                                           True)
        else:
            installed = False
    else:
        signal.signal(signal.SIGTERM,
                      lambda signum, frame : callback(*args, **kwargs))

    return installed

