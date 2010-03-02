"""
Defines ObjectPalette tree UI
"""
import wx
import cPickle as pickle

class ObjectPaletteUI(wx.Panel):
    def __init__(self, parent, editor):
        wx.Panel.__init__(self, parent)

        self.editor = editor

        self.palette = self.editor.objectPalette
        self.tree = wx.TreeCtrl(self)
        root = self.tree.AddRoot('Objects')
        self.addTreeNodes(root, self.palette.dataStruct, self.palette.dataKeys)
        self.SortTreeNodes(root)

        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(self.tree, 1, wx.EXPAND, 0)
        self.SetSizer(sizer); self.Layout()

        parentSizer = wx.BoxSizer(wx.VERTICAL)
        parentSizer.Add(self, 1, wx.EXPAND, 0)
        parent.SetSizer(parentSizer); parent.Layout()

        self.tree.Bind(wx.EVT_TREE_SEL_CHANGED, self.onSelected)
        self.tree.Bind(wx.EVT_TREE_BEGIN_DRAG, self.onBeginDrag)

    def traverse(self, parent, itemText):
        if itemText == self.tree.GetItemText(parent):
           return parent
        item, cookie = self.tree.GetFirstChild(parent)
        while item:
              # if the item was found - return it
              if itemText == self.tree.GetItemText(item):
                 return item

              # the tem was not found - checking if it has children
              if self.tree.ItemHasChildren(item):
                 # item has children - delving into it
                 child = self.traverse(item, itemText)
                 if child is not None:
                    return child

              # continue iteration to the next child
              item, cookie = self.tree.GetNextChild(parent, cookie)
        return None

    def addTreeNode(self, itemText, parentItem, items):
        newItem = wx.TreeItemId
        parentText = items[itemText]
        if parentText == self.palette.rootName:
           newItem = self.tree.AppendItem(parentItem, itemText)
           self.tree.SetItemPyData(newItem, itemText)
        else:
           item = self.traverse(parentItem, parentText)
           if item is None:
              item = self.addTreeNode(parentText, parentItem, items)

           newItem = self.tree.AppendItem(item, itemText)
           self.tree.SetItemPyData(newItem, itemText)

        return newItem

    def addTreeNodes(self, parentItem, items, itemKeys):
        for key in itemKeys:
            item = self.traverse(parentItem, key)
            if item is None:
               newItem = self.addTreeNode(key, parentItem, items)

    def SortTreeNodes(self, parent):
        self.tree.SortChildren(parent)
        item, cookie = self.tree.GetFirstChild(parent)
        while item:
              if self.tree.ItemHasChildren(item):
                 self.SortTreeNodes(item)

              # continue iteration to the next child
              item, cookie = self.tree.GetNextChild(parent, cookie)

    def onSelected(self, event):
        pass

    def onBeginDrag(self, event):
        item = event.GetItem()

        if item != self.tree.GetRootItem(): # prevent dragging root item
            text = self.tree.GetItemText(item)
            print "Starting drag'n'drop with %s..." % repr(text)

            tdo = wx.TextDataObject(text)
            tds = wx.DropSource(self.tree)
            tds.SetData(tdo)
            tds.DoDragDrop(True)
