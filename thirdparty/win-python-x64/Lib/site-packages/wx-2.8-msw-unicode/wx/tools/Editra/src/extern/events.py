#----------------------------------------------------------------------
# Name:        wxblox/events.py
# Purpose:     These mixins implement a push and pop menu/UI update event 
#              handler system at the wx.App level. This is useful for resolving
#              cases where multiple views may want to respond to an event 
#              (say, wx.ID_COPY) and where you also want a "default" handler 
#              for the event (and UI update status) when there is no active
#              view which wishes to handle the event.
#
# Author:      Kevin Ollivier
#
# Created:     -Mar-
# Copyright:   (c)  Kevin Ollivier
# Licence:     wxWindows license
#----------------------------------------------------------------------

import sys, os
import wx

class AppEventManager:
    ui_events = [
                    wx.ID_NEW, wx.ID_OPEN, wx.ID_CLOSE_ALL, wx.ID_CLOSE,
                    wx.ID_REVERT, wx.ID_SAVE, wx.ID_SAVEAS, wx.ID_UNDO,
                    wx.ID_REDO, wx.ID_PRINT, wx.ID_PRINT_SETUP, wx.ID_PREVIEW,
                    wx.ID_EXIT
                ]

    def __init__(self):
        pass

    def RegisterEvents(self):
        app = wx.GetApp()
        #app.AddHandlerForID(wx.ID_EXIT, self.OnExit)
        #app.AddHandlerForID(wx.ID_ABOUT, self.OnAbout)

        for eventID in self.ui_events:
            app.AddHandlerForID(eventID, self.ProcessEvent)
            app.AddUIHandlerForID(eventID, self.ProcessUpdateUIEvent)

class AppEventHandlerMixin:
    """
    The purpose of the AppEventHandlerMixin is to provide a centralized
    location to manage menu and toolbar events. In an IDE which may have
    any number of file editors and services open that may want to respond
    to certain menu and toolbar events (e.g. copy, paste, select all),
    we need this to efficiently make sure that the right handler is handling
    the event.

    To work with this system, views must call 
            Add(UI)HandlerForID(ID, handlerFunc)
    in their EVT_SET_FOCUS handler, and call Remove(UI)HandlerForID(ID) in their
    EVT_KILL_FOCUS handler.
    """

    def __init__(self):
        self.handlers = {}
        self.uihandlers = {}

        # When a view changes the handler, move the old one here.
        # Then "pop" the handler when the view loses the focus
        self.pushed_handlers = {}
        self.pushed_uihandlers = {}

    def AddHandlerForIDs(self, eventID_list, handlerFunc):
        for eventID in eventID_list:
            self.AddHandlerForID(eventID, handlerFunc)

    def AddHandlerForID(self, eventID, handlerFunc):
        self.Bind(wx.EVT_MENU, self.HandleEvent, id=eventID)

        if eventID in self.handlers:
            self.pushed_handlers[eventID] = self.handlers[eventID]

        self.handlers[eventID] = handlerFunc

    def AddUIHandlerForID(self, eventID, handlerFunc):
        self.Bind(wx.EVT_UPDATE_UI, self.HandleUpdateUIEvent, id=eventID)

        if eventID in self.uihandlers:
            self.pushed_uihandlers[eventID] = self.uihandlers[eventID]

        self.uihandlers[eventID] = handlerFunc

    def RemoveHandlerForIDs(self, eventID_list):
        for eventID in eventID_list:
            self.RemoveHandlerForID(eventID)

    def RemoveHandlerForID(self, eventID):
        self.Unbind(wx.EVT_MENU, id=eventID)
        self.handlers[eventID] = None

        if eventID in self.pushed_handlers:
            self.handlers[eventID] = self.pushed_handlers[eventID]

    def RemoveUIHandlerForID(self, eventID):
        self.Unbind(wx.EVT_UPDATE_UI, id=eventID)
        self.uihandlers[eventID] = None

        if eventID in self.pushed_uihandlers:
            self.uihandlers[eventID] = self.pushed_uihandlers[eventID]

    def HandleEvent(self, event):
        e_id = event.GetId()
        if e_id in self.handlers:
            handler = self.handlers[e_id]
            try:
                if handler:
                    return handler(event)
            except wx.PyDeadObjectError:
                self.RemoveHandlerForID(e_id)
        else:
            event.Skip()

        return False

    def HandleUpdateUIEvent(self, event):
        e_id = event.GetId()
        if e_id in self.uihandlers:
            handler = self.uihandlers[e_id]
            try:
                if handler:
                    return handler(event)
            except wx.PyDeadObjectError:
                self.RemoveUIHandlerForID(e_id)
        else:
            event.Skip()

        return False
