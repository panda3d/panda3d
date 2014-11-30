#----------------------------------------------------------------------------
# Name:        wx.lib.eventwatcher
# Purpose:     A widget that allows some or all events for a particular widget
#              to be captured and displayed.
#
# Author:      Robin Dunn
#
# Created:     21-Jan-2009
# RCS-ID:      $Id: $
# Copyright:   (c) 2009 by Total Control Software
# Licence:     wxWindows license
#----------------------------------------------------------------------------

"""
A widget and supporting classes for watching the events sent to some other widget.
"""

import wx
from wx.lib.mixins.listctrl import CheckListCtrlMixin

#----------------------------------------------------------------------------
# Helpers for building the data structures used for tracking the
# various event binders that are available

_eventBinders = None
_eventIdMap = None

def _buildModuleEventMap(module):
    count = 0
    for name in dir(module):
        if name.startswith('EVT_'):
            item = getattr(module, name)
            if isinstance(item, wx.PyEventBinder) and \
               len(item.evtType) == 1 and \
               item not in _eventBinders:
                    _eventBinders.append(item)
                    _eventIdMap[item.typeId] = name
                    count += 1
    return count


def buildWxEventMap():
    """
    Add the event binders from the main wx namespace.  This is called
    automatically from the EventWatcher.
    """
    global _eventBinders
    global _eventIdMap
    if _eventBinders is None:
        _eventBinders = list()
        _eventIdMap = dict()
        _buildModuleEventMap(wx)


def addModuleEvents(module):
    """
    Adds all the items in module that start with 'EVT_' to the event
    data structures used by the EventWatcher.
    """
    if _eventBinders is None:
        buildWxEventMap()
    return _buildModuleEventMap(module)
    

# Events that should not be watched by default
_noWatchList = [
    wx.EVT_PAINT, 
    wx.EVT_NC_PAINT,
    wx.EVT_ERASE_BACKGROUND,
    wx.EVT_IDLE, 
    wx.EVT_UPDATE_UI,
    wx.EVT_UPDATE_UI_RANGE,
    ]
OTHER_WIDTH = 250
    

def _makeSourceString(wdgt):
    if wdgt is None:
        return "None"
    else:
        return '%s "%s" (%d)' % (wdgt.__class__.__name__, wdgt.GetName(), wdgt.GetId())

def _makeAttribString(evt):
    "Find all the getters"
    attribs = ""
    for name in dir(evt):
        if (name.startswith('Get') or name.startswith('Is')) and \
               name not in [ 'GetEventObject',
                             'GetEventType',
                             'GetId',
                             'GetSkipped',
                             'GetTimestamp',
                             'GetClientData',
                             'GetClientObject',
                             ]:
            try:
                value = getattr(evt, name)()
                attribs += "%s : %s\n" % (name, value)
            except:
                pass
        
    return attribs.rstrip()
    
#----------------------------------------------------------------------------

class EventLog(wx.ListCtrl):
    """
    A virtual listctrl that displays information about the watched events.
    """
    def __init__(self, *args, **kw):
        kw['style'] = wx.LC_REPORT|wx.LC_SINGLE_SEL|wx.LC_VIRTUAL|wx.LC_HRULES|wx.LC_VRULES
        wx.ListCtrl.__init__(self, *args, **kw)
        self.clear()
        
        if 'wxMac' in wx.PlatformInfo:
            self.SetWindowVariant(wx.WINDOW_VARIANT_SMALL)
        
        self.InsertColumn(0, "#", format=wx.LIST_FORMAT_RIGHT, width=50)
        self.InsertColumn(1, "Event", width=200)
        self.InsertColumn(2, "Source", width=200)
        
        self.SetMinSize((450+wx.SystemSettings.GetMetric(wx.SYS_VSCROLL_X), 450))
        self.Bind(wx.EVT_LIST_ITEM_SELECTED, self.onItemSelected)
        self.Bind(wx.EVT_LIST_ITEM_ACTIVATED, self.onItemActivated)
        self.Bind(wx.EVT_LIST_ITEM_RIGHT_CLICK, self.onItemActivated)
        
    def append(self, evt):
        evtName = _eventIdMap.get(evt.GetEventType(), None)
        if evtName is None:
            evtName = 'Unknown: %d' % evt.GetEventType()
        source = _makeSourceString(evt.GetEventObject())
        attribs = _makeAttribString(evt)

        lastIsSelected = self.currItem == len(self.data)-1
        self.data.append( (evtName, source, attribs) )
        
        count = len(self.data)
        self.SetItemCount(count)
        self.RefreshItem(count-1)
        if lastIsSelected:
            self.Select(count-1)
            self.EnsureVisible(count-1)
            
    def clear(self):
        self.data = []
        self.SetItemCount(0)
        self.currItem = -1
        self.Refresh()
                
    def OnGetItemText(self, item, col):
        if col == 0:
            val = str(item+1)
        else:
            val = self.data[item][col-1]
        return val
    
    def OnGetItemAttr(self, item):  return None
    def OnGetItemImage(self, item): return -1
    
    def onItemSelected(self, evt):
        self.currItem = evt.GetIndex()

    def onItemActivated(self, evt):
        idx = evt.GetIndex()
        text = self.data[idx][2]
        wx.CallAfter(wx.TipWindow, self, text, OTHER_WIDTH)

#----------------------------------------------------------------------------


class EventChooser(wx.Panel):
    """
    Panel with CheckListCtrl for selecting which events will be watched.
    """
    
    class EventChooserLC(wx.ListCtrl, CheckListCtrlMixin):
        def __init__(self, parent):
            wx.ListCtrl.__init__(self, parent, 
                                 style=wx.LC_REPORT|wx.LC_SINGLE_SEL|wx.LC_HRULES|wx.LC_VRULES)
            CheckListCtrlMixin.__init__(self)
            if 'wxMac' in wx.PlatformInfo:
                self.SetWindowVariant(wx.WINDOW_VARIANT_SMALL)
            
        # this is called by the base class when an item is checked/unchecked
        def OnCheckItem(self, index, flag):
            self.Parent.OnCheckItem(index, flag)
            
            
    def __init__(self, *args, **kw):
        wx.Panel.__init__(self, *args, **kw)
        self.updateCallback = lambda: None
        self.doUpdate = True
        self.lc = EventChooser.EventChooserLC(self)
        btn1 = wx.Button(self, -1, "All")
        btn2 = wx.Button(self, -1, "None")
        btn1.SetToolTipString("Check all events")
        btn2.SetToolTipString("Uncheck all events")

        self.Bind(wx.EVT_BUTTON, self.onCheckAll, btn1)
        self.Bind(wx.EVT_BUTTON, self.onUncheckAll, btn2)        
        
        self.Bind(wx.EVT_LIST_ITEM_ACTIVATED, self.onItemActivated, self.lc)
        self.lc.InsertColumn(0, "Binder", width=OTHER_WIDTH)        
        
        btnSizer = wx.BoxSizer(wx.HORIZONTAL)
        btnSizer.Add(btn1, 0, wx.ALL, 5)
        btnSizer.Add(btn2, 0, wx.ALL, 5)
        
        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(self.lc, 1, wx.EXPAND)
        sizer.Add(btnSizer)
        self.SetSizer(sizer)
        
        
    def setUpdateCallback(self, func):
        self.updateCallback = func
    
    def setWatchList(self, watchList):
        self.doUpdate = False
        self.watchList = watchList
        self.lc.DeleteAllItems()
        for count, (item, flag) in enumerate(watchList):
            typeId = item.typeId
            text = _eventIdMap.get(typeId, "[Unknown]")
            self.lc.InsertStringItem(count, text)
            self.lc.SetItemData(count, count)
            if flag:
                self.lc.CheckItem(count)
        self.lc.SortItems(self.sortCompare)
        self.doUpdate = True
        self.updateCallback()

        
    def OnCheckItem(self, index, flag):
        index = self.lc.GetItemData(index)
        item, f = self.watchList[index]
        self.watchList[index] = (item, flag)
        if self.doUpdate:
            self.updateCallback()
            
            
    def onItemActivated(self, evt):
        self.lc.ToggleItem(evt.m_itemIndex)

    def onCheckAll(self, evt):
        self.doUpdate = False
        for idx in range(self.lc.GetItemCount()):
            self.lc.CheckItem(idx, True)
        self.doUpdate = True
        self.updateCallback()
    
    def onUncheckAll(self, evt):
        self.doUpdate = False
        for idx in range(self.lc.GetItemCount()):
            self.lc.CheckItem(idx, False)
        self.doUpdate = True
        self.updateCallback()

    
    def sortCompare(self, data1, data2):
        item1 = self.watchList[data1][0]
        item2 = self.watchList[data2][0]
        text1 = _eventIdMap.get(item1.typeId)
        text2 = _eventIdMap.get(item2.typeId)
        return cmp(text1, text2)
        
        
#----------------------------------------------------------------------------

class EventWatcher(wx.Frame):
    """
    A frame that will catch and display al events sent to some widget.
    """
    def __init__(self, *args, **kw):
        wx.Frame.__init__(self, *args, **kw)
        self.SetTitle("EventWatcher")
        self.SetExtraStyle(wx.WS_EX_BLOCK_EVENTS)
        self._watchedWidget = None

        buildWxEventMap()
        self.buildWatchList(_noWatchList)
        
        # Make the widgets
        self.splitter = wx.SplitterWindow(self)
        panel = wx.Panel(self.splitter)
        self.splitter.Initialize(panel)
        self.log = EventLog(panel)
        clearBtn = wx.Button(panel, -1, "Clear")
        addBtn = wx.Button(panel, -1, "Add Module")
        watchBtn = wx.ToggleButton(panel, -1, "Watch")
        watchBtn.SetValue(True)
        selectBtn = wx.ToggleButton(panel, -1, ">>>")
        self.selectBtn = selectBtn
        
        clearBtn.SetToolTipString("Clear the event log")
        addBtn.SetToolTipString("Add the event binders in an additional package or module to the watcher")
        watchBtn.SetToolTipString("Toggle the watching of events")
        selectBtn.SetToolTipString("Show/hide the list of events to be logged")
        
        # Do the layout
        sizer = wx.BoxSizer(wx.VERTICAL)
        btnSizer = wx.BoxSizer(wx.HORIZONTAL)
        btnSizer.Add(clearBtn, 0, wx.RIGHT, 5)
        btnSizer.Add(addBtn, 0, wx.RIGHT, 5)
        btnSizer.Add((1,1), 1)
        btnSizer.Add(watchBtn, 0, wx.RIGHT, 5)
        btnSizer.Add((1,1), 1)
        btnSizer.Add(selectBtn, 0, wx.RIGHT, 5)
        sizer.Add(self.log, 1, wx.EXPAND)
        sizer.Add(btnSizer, 0, wx.EXPAND|wx.ALL, 5)
        panel.SetSizer(sizer)
        self.Sizer = wx.BoxSizer()
        self.Sizer.Add(self.splitter, 1, wx.EXPAND)
        self.Fit()
        
        # Bind events
        self.Bind(wx.EVT_CLOSE, self.onCloseWindow)
        self.Bind(wx.EVT_BUTTON, self.onClear, clearBtn)
        self.Bind(wx.EVT_BUTTON, self.onAddModule, addBtn)
        self.Bind(wx.EVT_TOGGLEBUTTON, self.onToggleWatch, watchBtn)
        self.Bind(wx.EVT_TOGGLEBUTTON, self.onToggleSelectEvents, selectBtn)
        
            
        
    def watch(self, widget):
        assert self._watchedWidget is None, "Can only watch one widget at a time"
        self.SetTitle("EventWatcher for " + _makeSourceString(widget))
        for evtBinder, flag in self._watchedEvents:
            if flag:
                widget.Bind(evtBinder, self.onWatchedEvent)
        self._watchedWidget = widget
        
                
    def unwatch(self):
        self.SetTitle("EventWatcher")
        if self._watchedWidget:
            for evtBinder, flag in self._watchedEvents:
                self._watchedWidget.Unbind(evtBinder, handler=self.onWatchedEvent)
        self._watchedWidget = None
    
        
    def updateBindings(self):
        widget = self._watchedWidget
        self.unwatch()
        self.watch(widget)
        
        
    def onWatchedEvent(self, evt):     
        if self:
            self.log.append(evt)
        evt.Skip()
        
    def buildWatchList(self, exclusions):
        # This is a list of (PyEventBinder, flag) tuples where the flag indicates
        # whether to bind that event or not. By default all execpt those in
        # the _noWatchList wil be set to be watched.
        self._watchedEvents = list()
        for item in _eventBinders:
            self._watchedEvents.append( (item, item not in exclusions) )
            
    def onCloseWindow(self, evt):
        self.unwatch()
        evt.Skip()
        
    def onClear(self, evt):
        self.log.clear()
        
    def onAddModule(self, evt):
        try:
            dlg = wx.TextEntryDialog(
                self, 
                "Enter the package or module name to be scanned for \"EVT_\" event binders.",
                "Add Module")
            if dlg.ShowModal() == wx.ID_OK:
                modname = dlg.GetValue()
                try:
                    # Passing a non-empty fromlist will cause __import__ to
                    # return the imported submodule if a dotted name is passed.
                    module = __import__(modname, fromlist=[0])
                except ImportError:
                    wx.MessageBox("Unable to import \"%s\"" % modname,
                                  "Error")
                    return
                count = addModuleEvents(module)
                wx.MessageBox("%d new event binders found" % count,
                              "Success")
                
                # Now unwatch and re-watch so we can get the new events bound
                self.updateBindings()
        finally:    
            dlg.Destroy()
    
            
    def onToggleWatch(self, evt):
        if evt.Checked():
            self.watch(self._unwatchedWidget)
            self._unwatchedWidget = None
        else:
            self._unwatchedWidget = self._watchedWidget
            self.unwatch()

    
    def onToggleSelectEvents(self, evt):
        if evt.Checked():
            self.selectBtn.SetLabel("<<<")
            self._selectList = EventChooser(self.splitter)
            self._selectList.setUpdateCallback(self.updateBindings)
            self._selectList.setWatchList(self._watchedEvents)
            
            self.SetSize(self.GetSize() + (OTHER_WIDTH,0))
            self.splitter.SplitVertically(self.splitter.GetWindow1(), 
                                          self._selectList,
                                          -OTHER_WIDTH)
        else:
            self.selectBtn.SetLabel(">>>")
            sashPos = self.splitter.GetSashPosition()
            self.splitter.Unsplit()
            self._selectList.Destroy()
            cs = self.GetClientSize()
            self.SetClientSize((sashPos, cs.height))
        
#----------------------------------------------------------------------------
        
if __name__ == '__main__':
    app = wx.App(redirect=False)
    frm = wx.Frame(None, title="Test Frame")
    pnl = wx.Panel(frm)
    txt = wx.TextCtrl(pnl, -1, "text", pos=(20,20))
    btn = wx.Button(pnl, -1, "button", pos=(20,50))
    frm.Show()
    
    ewf=EventWatcher(frm)
    ewf.watch(frm)
    ewf.Show()
    
    #import wx.lib.inspection
    #wx.lib.inspection.InspectionTool().Show()
    
    app.MainLoop()
    
