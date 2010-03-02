"""
Defines ObjectPalette tree UI
"""
import wx
import cPickle as pickle

class ObjectPaletteTreeCtrl(wx.TreeCtrl):
    def __init__(self, parent):
        wx.TreeCtrl.__init__(self, parent)
        
        self.paletteUI = parent

    def OnCompareItems(self, item1, item2):
        data1 = self.GetItemText(item1)
        data2 = self.GetItemText(item2)
        if self.paletteUI.opSort == self.paletteUI.opSortAlpha:
           return cmp(data1, data2)
        else:
           index1 = self.paletteUI.palette.dataKeys.index(data1)
           index2 = self.paletteUI.palette.dataKeys.index(data2)
           return cmp(index1, index2)

class ObjectPaletteUI(wx.Panel):
    def __init__(self, parent, editor):
        wx.Panel.__init__(self, parent)

        self.editor = editor

        self.palette = self.editor.objectPalette
        self.tree = ObjectPaletteTreeCtrl(self)
        root = self.tree.AddRoot('Objects')
        self.addTreeNodes(root, self.palette.dataStruct, self.palette.dataKeys)

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
        self.SortTreeNodes(self.tree.GetRootItem())
