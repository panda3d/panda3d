"""
Defines ObjectPalette tree UI
"""
import wx
from .PaletteTreeCtrl import *


class ObjectPaletteUI(wx.Panel):
    def __init__(self, parent, editor):
        wx.Panel.__init__(self, parent)

        self.editor = editor

        self.palette = self.editor.objectPalette
        self.tree = PaletteTreeCtrl(self, treeStyle=wx.TR_DEFAULT_STYLE, rootName='Objects')

        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(self.tree, 1, wx.EXPAND, 0)
        self.SetSizer(sizer); self.Layout()

        parentSizer = wx.BoxSizer(wx.VERTICAL)
        parentSizer.Add(self, 1, wx.EXPAND, 0)
        parent.SetSizer(parentSizer); parent.Layout()

        self.opSortAlpha = "Sort Alphabetical Order"
        self.opSortOrig  = "Sort Original Order"
        self.opSort = self.opSortOrig

        self.menuItems = list()
        self.menuItems.append(self.opSortAlpha)
        self.menuItems.append(self.opSortOrig)

        self.popupmenu = wx.Menu()
        for item in self.menuItems:
            menuItem = self.popupmenu.Append(-1, item)
            self.Bind(wx.EVT_MENU, self.onPopupItemSelected, menuItem)
        self.Bind(wx.EVT_CONTEXT_MENU, self.onShowPopup)

        self.tree.Bind(wx.EVT_TREE_SEL_CHANGED, self.onSelected)

    def populate(self):
        self.tree.addTreeNodes(self.tree.GetRootItem(), self.palette.rootName, self.palette.dataStruct, self.palette.dataKeys)

    def onSelected(self, event):
        pass

    def onShowPopup(self, event):
        pos = event.GetPosition()
        pos = self.ScreenToClient(pos)
        self.PopupMenu(self.popupmenu, pos)

    def onPopupItemSelected(self, event):
        menuItem = self.popupmenu.FindItemById(event.GetId())
        text = menuItem.GetText()
        if text == self.opSortAlpha:
           self.opSort = self.opSortAlpha
        elif text == self.opSortOrig:
           self.opSort = self.opSortOrig
        self.tree.SortTreeNodes(self.tree.GetRootItem())

    def compareItems(self, item1, item2):
        data1 = self.tree.GetItemText(item1)
        data2 = self.tree.GetItemText(item2)
        if self.opSort == self.opSortAlpha:
            return (data1 > data2) - (data1 < data2)
        else:
            index1 = self.palette.dataKeys.index(data1)
            index2 = self.palette.dataKeys.index(data2)
            return (index1 > index2) - (index1 < index2)

    def getSelected(self):
        return self.tree.GetItemPyData(self.tree.GetSelection())
