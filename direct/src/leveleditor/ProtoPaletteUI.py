"""
Defines ProtoPalette tree UI
"""
import wx
import os
import cPickle as pickl
from pandac.PandaModules import *
from ObjectPaletteBase import *

class FileDrop(wx.FileDropTarget):
    def __init__(self, editor):
        wx.FileDropTarget.__init__(self)
        self.editor = editor

    def OnDropFiles(self, x, y, filenames):
        for filename in filenames:
            name = os.path.basename(filename)

            if self.editor.protoPalette.findItem(name):
                print 'This model already exists in ProtoPalette!'
                return

            modelname = Filename.fromOsSpecific(filename).getFullpath()
            itemData = ObjectBase(name=name, model=modelname, actor=True)
            self.editor.protoPalette.add(itemData)
            newItem = self.editor.ui.protoPaletteUI.tree.AppendItem(self.editor.ui.protoPaletteUI.root, name)
            self.editor.ui.protoPaletteUI.tree.SetItemPyData(newItem, itemData)
            self.editor.ui.protoPaletteUI.tree.ScrollTo(newItem)

class ProtoPaletteUI(wx.Panel):
    def __init__(self, parent, editor):
        wx.Panel.__init__(self, parent)

        self.editor = editor
        self.palette = self.editor.protoPalette
        self.tree = wx.TreeCtrl(self)
        self.root = self.tree.AddRoot('Proto Objects')
        self.addTreeNodes(self.root, self.palette.data)

        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(self.tree, 1, wx.EXPAND, 0)
        self.SetSizer(sizer); self.Layout()

        parentSizer = wx.BoxSizer(wx.VERTICAL)
        parentSizer.Add(self, 1, wx.EXPAND, 0)
        parent.SetSizer(parentSizer); parent.Layout()

        self.tree.Bind(wx.EVT_TREE_SEL_CHANGED, self.onSelected)
        self.tree.Bind(wx.EVT_TREE_BEGIN_DRAG, self.onBeginDrag)

        self.SetDropTarget(FileDrop(self.editor))

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
