###############################################################################
# Name: ed_event.py                                                           #
# Purpose: Custom events used by Editra                                       #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
Provides custom events for the editors controls/objects to utilize

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: ed_event.py 63789 2010-03-30 02:25:17Z CJP $"
__revision__ = "$Revision: 63789 $"

#-----------------------------------------------------------------------------#
# Dependencies
import wx

#-----------------------------------------------------------------------------#

edEVT_UPDATE_TEXT = wx.NewEventType()
EVT_UPDATE_TEXT = wx.PyEventBinder(edEVT_UPDATE_TEXT, 1)
class UpdateTextEvent(wx.PyCommandEvent):
    """Event to signal that text needs updating"""
    def __init__(self, etype, eid, value=None):
        """Creates the event object"""
        wx.PyCommandEvent.__init__(self, etype, eid)
        self._value = value

    def GetValue(self):
        """Returns the value from the event.
        @return: the value of this event

        """
        return self._value

#--------------------------------------------------------------------------#

edEVT_NOTIFY = wx.NewEventType()
EVT_NOTIFY = wx.PyEventBinder(edEVT_NOTIFY, 1)
class NotificationEvent(UpdateTextEvent):
    """General notification event"""
    def __init__(self, etype, eid, value=None, obj=None):
        UpdateTextEvent.__init__(self, etype, eid, value)
        self.SetEventObject(obj)

#--------------------------------------------------------------------------#

edEVT_MAINWINDOW_EXIT = wx.NewEventType()
EVT_MAINWINDOW_EXIT = wx.PyEventBinder(edEVT_MAINWINDOW_EXIT, 1)
class MainWindowExitEvent(wx.PyCommandEvent):
    """Event to signal that the main window is exiting"""
    pass

#--------------------------------------------------------------------------#

edEVT_STATUS = wx.NewEventType()
EVT_STATUS = wx.PyEventBinder(edEVT_STATUS, 1)
class StatusEvent(wx.PyCommandEvent):
    """Event for posting status events"""
    def __init__(self, etype, eid, msg=None, sec=0):
        """Create an event that can be used to post status messages
        to the main windows status bar.
        @param etype: The type of event to create
        @param eid: The event id
        @keyword msg: The status message to post with the event
        @keyword sec: The section of the status bar to post message to

        """
        wx.PyCommandEvent.__init__(self, etype, eid)
        self._msg = msg
        self._sec = sec

    def GetMessage(self):
        """Returns the value from the event.
        @return: the value of this event

        """
        return self._msg

    def GetSection(self):
        """Returns the messages posting section
        @return: int zero based index of where to post to statusbar

        """
        return self._sec
