"""
Defines ProtoObjs List UI
"""
import wx
import os

from panda3d.core import *
from .ProtoObjs import *

class ProtoDropTarget(wx.PyDropTarget):
   """Implements drop target functionality to receive files, bitmaps and text"""
   def __init__(self, ui):
       wx.PyDropTarget.__init__(self)
       self.ui = ui
       self.do = wx.DataObjectComposite()  # the dataobject that gets filled with the appropriate data
       self.filedo = wx.FileDataObject()
       self.textdo = wx.TextDataObject()
       self.bmpdo = wx.BitmapDataObject()
       self.do.Add(self.filedo)
       self.do.Add(self.bmpdo)
       self.do.Add(self.textdo)
       self.SetDataObject(self.do)

   def OnData(self, x, y, d):
       """
       Handles drag/dropping files/text or a bitmap
       """
       if self.GetData():
          df = self.do.GetReceivedFormat().GetType()
          if df in [wx.DF_UNICODETEXT, wx.DF_TEXT]:
             text = self.textdo.GetText()
             # self.editor.ui.protoFontsUI.tree.ChangeHierarchy(text, x, y)

          elif df == wx.DF_FILENAME:
               for name in self.filedo.GetFilenames():
                   self.ui.AquireFile(name)

          elif df == wx.DF_BITMAP:
               bmp = self.bmpdo.GetBitmap()

       return d  # you must return this

class ProtoObjsUI(wx.Panel):
    def __init__(self, parent, editor, protoObjs, supportedExts):
        wx.Panel.__init__(self, parent)

        self.editor = editor
        self.protoObjs = protoObjs
        self.supportedExts = supportedExts

        self.llist = wx.ListCtrl(self, -1, style=wx.LC_REPORT)
        self.llist.InsertColumn(0, "Files")

        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(self.llist, 1, wx.EXPAND, 0)
        self.SetSizer(sizer); self.Layout()

        parentSizer = wx.BoxSizer(wx.VERTICAL)
        parentSizer.Add(self, 1, wx.EXPAND, 0)
        parent.SetSizer(parentSizer); parent.Layout()

        self.opDelete = "Delete"
        self.menuItems = list()
        self.menuItems.append(self.opDelete)

        self.popupmenu = wx.Menu()
        for item in self.menuItems:
            menuItem = self.popupmenu.Append(-1, item)
            self.Bind(wx.EVT_MENU, self.onPopupItemSelected, menuItem)
        self.Bind(wx.EVT_CONTEXT_MENU, self.onShowPopup)

        self.SetDropTarget(ProtoDropTarget(self))

    def populate(self):
        for key in list(self.protoObjs.data.keys()):
            self.add(self.protoObjs.data[key])

    # All subclasses should implement this method
    def addObj(self, filename):
        raise NotImplementedError

    def onPopupItemSelected(self, event):
        menuItem = self.popupmenu.FindItemById(event.GetId())
        text = menuItem.GetText()
        if text == self.opDelete:
           self.remove()

    def onShowPopup(self, event):
        pos = event.GetPosition()
        pos = self.ScreenToClient(pos)
        self.PopupMenu(self.popupmenu, pos)

    def findLabel(self, text):
        found = False
        for index in range(self.llist.GetItemCount()):
            itemtext = self.llist.GetItemText(index)
            if itemtext == text:
               return True
        return found

    def removeItem(self, index):
        if index != -1:
           key = self.llist.GetItemText(index)
           del(self.protoObjs.data[key])
           item = self.llist.DeleteItem(index)

    def remove(self):
        index = self.llist.GetFirstSelected()
        self.removeItem(index)

    def add(self, filename):
        name = os.path.basename(filename)
        for ext in self.supportedExts:
            if name.upper().endswith(ext.upper()):
               try:
                  index = self.llist.InsertStringItem(self.llist.GetItemCount(), name)
                  self.protoObjs.data[name]= filename
                  self.addObj(filename)
               except:
                  pass
               break

    def addNewItem(self, result):
       ProtoObjsUI.AquireFile(self, result[1])

    def AquireFile(self, filename):
        label = self.findLabel(filename)
        if label:
           self.removeItem(label)
        filenameFull = Filename.fromOsSpecific(filename).getFullpath()
        self.add(filenameFull)
