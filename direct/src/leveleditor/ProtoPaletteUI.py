"""
Defines ProtoPalette tree UI
"""
import wx
import os
from pandac.PandaModules import *
from .PaletteTreeCtrl import *

class UniversalDropTarget(wx.PyDropTarget):
   """Implements drop target functionality to receive files, bitmaps and text"""
   def __init__(self, editor):
       wx.PyDropTarget.__init__(self)
       self.editor = editor
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
             self.editor.ui.protoPaletteUI.tree.ChangeHierarchy(text, x, y)

          elif df == wx.DF_FILENAME:
               for name in self.filedo.GetFilenames():
                   self.editor.ui.protoPaletteUI.AquireFile(name)

          elif df == wx.DF_BITMAP:
               bmp = self.bmpdo.GetBitmap()

       return d  # you must return this

class ProtoPaletteUI(wx.Panel):
    def __init__(self, parent, editor):
        wx.Panel.__init__(self, parent)

        self.editor = editor
        self.palette = self.editor.protoPalette
        self.tree = PaletteTreeCtrl(self, treeStyle=wx.TR_EDIT_LABELS|wx.TR_DEFAULT_STYLE, rootName="Proto Objects")

        self.editorTxt = "Proto Objects Editor"

        self.opSortAlpha = "Sort Alphabetical Order"
        self.opSortOrig  = "Sort Original Order"
        self.opSort = self.opSortOrig

        self.opAdd    = "Add Group"
        self.opDelete = "Delete"

        self.menuItemsGen = list()
        self.menuItemsGen.append(self.opSortAlpha)
        self.menuItemsGen.append(self.opSortOrig)
        self.menuItemsGen.append(self.opAdd)

        self.menuItemsSel = list()
        self.menuItemsSel.append(self.opSortAlpha)
        self.menuItemsSel.append(self.opSortOrig)
        self.menuItemsSel.append(self.opAdd)
        self.menuItemsSel.append(self.opDelete)

        self.popupmenu = wx.Menu()
        for item in self.menuItemsGen:
            menuItem = self.popupmenu.Append(-1, item)
            self.Bind(wx.EVT_MENU, self.onPopupItemSelected, menuItem)

        self.Bind(wx.EVT_CONTEXT_MENU, self.onShowPopup)

        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(self.tree, 1, wx.EXPAND, 0)
        self.SetSizer(sizer); self.Layout()

        parentSizer = wx.BoxSizer(wx.VERTICAL)
        parentSizer.Add(self, 1, wx.EXPAND, 0)
        parent.SetSizer(parentSizer); parent.Layout()

        self.tree.Bind(wx.EVT_TREE_BEGIN_LABEL_EDIT, self.OnBeginLabelEdit)
        self.tree.Bind(wx.EVT_TREE_END_LABEL_EDIT, self.OnEndLabelEdit)

        self.SetDropTarget(UniversalDropTarget(self.editor))

    def populate(self):
        dataStructKeys = list(self.palette.dataStruct.keys())
        self.tree.addTreeNodes(self.tree.GetRootItem(), self.palette.rootName, self.palette.dataStruct, dataStructKeys)

    def OnBeginLabelEdit(self, event):
        self.editor.ui.bindKeyEvents(False)

    def OnEndLabelEdit(self, event):
        #import pdb;set_trace()
        item = event.GetItem()
        if item != self.tree.GetRootItem():
            newLabel = event.GetLabel()
            if self.tree.traverse(self.tree.GetRootItem(), newLabel) is None:
               oldLabel = self.tree.GetItemText(item)
               if isinstance(self.editor.protoPalette.findItem(oldLabel), ObjectBase):
                  event.Veto()
                  wx.MessageBox("Only groups allowed to be renamed", self.editorTxt, wx.OK|wx.ICON_EXCLAMATION)
               elif not self.editor.protoPalette.rename(oldLabel, newLabel):
                  event.Veto()
                  wx.MessageBox("Label '%s' is not allowed" % newLabel, self.editorTxt, wx.OK|wx.ICON_EXCLAMATION)
            else:
               event.Veto()
               wx.MessageBox("There is already an item labled '%s'" % newLabel, self.editorTxt, wx.OK|wx.ICON_EXCLAMATION)
        else:
            event.Veto()
            wx.MessageBox("'%s' renaming is not allowed" % self.tree.rootName, self.editorTxt, wx.OK|wx.ICON_EXCLAMATION)
        self.editor.ui.bindKeyEvents(True)


    def menuAppendGenItems(self):
        for item in self.menuItemsGen:
            menuItem = self.popupmenu.Append(-1, item)
            self.Bind(wx.EVT_MENU, self.onPopupItemSelected, menuItem)

    def menuAppendSelItems(self):
        for item in self.menuItemsSel:
            menuItem = self.popupmenu.Append(-1, item)
            self.Bind(wx.EVT_MENU, self.onPopupItemSelected, menuItem)

    def onShowPopup(self, event):
        pos = event.GetPosition()
        pos = self.ScreenToClient(pos)

        for menuItem in self.popupmenu.GetMenuItems():
            self.popupmenu.RemoveItem(menuItem)

        hitItem, flags = self.tree.HitTest(pos)
        if hitItem.IsOk():
           itemText = self.tree.GetItemText(hitItem)
           if itemText != self.tree.rootName:
              self.menuAppendSelItems()
           else:
              self.menuAppendGenItems()
        else:
           self.menuAppendGenItems()

        self.PopupMenu(self.popupmenu, pos)

    def onPopupItemSelected(self, event):
        menuItem = self.popupmenu.FindItemById(event.GetId())
        text = menuItem.GetText()
        if text == self.opAdd:
           self.tree.AddGroup()
        elif text == self.opDelete:
           self.tree.DeleteSelected()
        elif text == self.opSortAlpha:
           self.opSort = self.opSortAlpha
           self.tree.SortTreeNodes(self.tree.GetRootItem())
        elif text == self.opSortOrig:
           self.opSort = self.opSortOrig
           self.tree.SortTreeNodes(self.tree.GetRootItem())

    def AquireFile(self, filename):
        name = os.path.basename(filename)

        if self.editor.protoPalette.findItem(name):
           item = self.tree.traverse(self.tree.root, name)
           if item:
              self.tree.DeleteItem(item)

        modelname = Filename.fromOsSpecific(filename).getFullpath()
        if modelname.endswith('.mb') or\
           modelname.endswith('.ma'):
            self.editor.convertMaya(modelname, self.addNewItem)
            return

        itemData = ObjectBase(name=name, model=modelname, actor=True)
        self.editor.protoPalette.add(itemData)

        newItem = self.tree.AppendItem(self.editor.ui.protoPaletteUI.tree.root, name)
        self.tree.SetItemPyData(newItem, itemData)
        self.tree.ScrollTo(newItem)

    def addNewItem(self, result):
       if len(result) == 2:
          itemData = ObjectBase(name=result[0], model=result[1], actor=False)
       elif len(result) == 3:
          itemData = ObjectBase(name=result[0], model=result[1], anims=[result[2]], actor=True)
       else:
          return
       self.palette.add(itemData)
       newItem = self.tree.AppendItem(self.tree.root, itemData.name)
       self.tree.SetItemPyData(newItem, itemData)
       self.tree.ScrollTo(newItem)

    def compareItems(self, item1, item2):
        data1 = self.tree.GetItemText(item1)
        data2 = self.tree.GetItemText(item2)
        if self.opSort == self.opSortAlpha:
            return (data1 > data2) - (data1 < data2)
        else:
            items = list(self.palette.data.keys())
            index1 = items.index(data1)
            index2 = items.index(data2)
            return (index1 > index2) - (index1 < index2)
