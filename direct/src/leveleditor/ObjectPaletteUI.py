"""
Defines ObjectPalette tree UI
"""
import wx
import cPickle as pickle
from ObjectPalette import *

class ObjectPaletteUI(wx.Panel):
    def __init__(self, parent, editor):
        wx.Panel.__init__(self, parent)

        self.editor = editor

        self.palette = self.editor.objectPalette
        self.tree = wx.TreeCtrl(self)
        root = self.tree.AddRoot('Objects')
        self.addTreeNodes(root, self.palette.data)

        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(self.tree, 1, wx.EXPAND, 0)
        self.SetSizer(sizer); self.Layout()

        parentSizer = wx.BoxSizer(wx.VERTICAL)
        parentSizer.Add(self, 1, wx.EXPAND, 0)
        parent.SetSizer(parentSizer); parent.Layout()

        self.tree.Bind(wx.EVT_TREE_SEL_CHANGED, self.onSelected)
        self.tree.Bind(wx.EVT_TREE_BEGIN_DRAG, self.onBeginDrag)

    def addTreeNodes(self, parentItem, items):
        for key in items.keys():
            newItem = self.tree.AppendItem(parentItem, key)
            if type(items[key]) == dict:
                self.addTreeNodes(newItem, items[key])
            else:
                self.tree.SetItemPyData(newItem, items[key])

    def onSelected(self, event):
        data = self.tree.GetItemPyData(event.GetItem())
        if data:
            print data.properties

    def onBeginDrag(self, event):
        item = event.GetItem()

        if item != self.tree.GetRootItem(): # prevent dragging root item
            text = self.tree.GetItemText(item)
            print "Starting drag'n'drop with %s..." % repr(text)

            tdo = wx.TextDataObject(text)
            tds = wx.DropSource(self.tree)
            tds.SetData(tdo)
            tds.DoDragDrop(True)
