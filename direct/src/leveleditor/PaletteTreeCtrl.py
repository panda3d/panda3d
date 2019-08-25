"""
Defines Palette tree control
"""
import wx
from .ObjectPaletteBase import *


class PaletteTreeCtrl(wx.TreeCtrl):
    def __init__(self, parent, treeStyle, rootName):
        wx.TreeCtrl.__init__(self, parent, style=treeStyle)

        self.rootName = rootName
        self.root = self.AddRoot(self.rootName)

        self.paletteUI = parent

        self.opSortAlpha = "Sort Alphabetical Order"
        self.opSortOrig  = "Sort Original Order"
        self.opSort = self.opSortOrig

        self.Bind(wx.EVT_TREE_BEGIN_DRAG, self.onBeginDrag)

    def OnCompareItems(self, item1, item2):
        return self.paletteUI.compareItems(item1, item2)

    def SortTreeNodes(self, parent):
        self.SortChildren(parent)
        item, cookie = self.GetFirstChild(parent)
        while item:
              if self.ItemHasChildren(item):
                 self.SortTreeNodes(item)

              # continue iteration to the next child
              item, cookie = self.GetNextChild(parent, cookie)

    def addTreeNodes(self, parentItem, parentItemName, items, itemKeys):
        roots = []
        rootItems = []
        for key in itemKeys:
            if parentItemName == items[key]:
               roots.append(key)
        for root in roots:
            newItem = self.AppendItem(parentItem, root)
            self.SetItemData(newItem, root)
            rootItems.append(newItem)
            itemKeys.remove(root)
        for rootItem in rootItems:
            self.addTreeNodes(rootItem, self.GetItemText(rootItem), items, itemKeys)

    def traverse(self, parent, itemText):
        if itemText == self.GetItemText(parent):
           return parent
        item, cookie = self.GetFirstChild(parent)
        while item:
              # if the item was found - return it
              if itemText == self.GetItemText(item):
                 return item

              # the tem was not found - checking if it has children
              if self.ItemHasChildren(item):
                 # item has children - delving into it
                 child = self.traverse(item, itemText)
                 if child is not None:
                    return child

              # continue iteration to the next child
              item, cookie = self.GetNextChild(parent, cookie)
        return None

    def AddGroup(self):
        #import pdb;set_trace()
        parent = self.GetSelection()
        if parent is None:
            parent = self.GetRootItem()

        i = 1
        namestr = "Group%s"%(i)
        found = self.traverse(self.GetRootItem(), namestr)
        while found:
              i = i + 1
              namestr = "Group%s"%(i)
              found = self.traverse(self.GetRootItem(), namestr)

        newItem = self.AppendItem(parent, namestr)
        itemData = ObjectGen(name=namestr)
        parentName = self.GetItemText(parent)
        if parentName == self.rootName:
           self.paletteUI.palette.add(itemData)
        else:
           self.paletteUI.palette.add(itemData, parentName)
        self.SetItemPyData(newItem, itemData)

        self.Expand(self.GetRootItem())
        self.ScrollTo(newItem)

    def DeleteItem(self, item):
        itemText = self.GetItemText(item)
        if item and itemText != self.rootName:
           self.Delete(item)
           self.paletteUI.palette.delete(itemText)

    def DeleteSelected(self):
        item = self.GetSelection()
        self.DeleteItem(item)

    def ReParent(self, parent, newParent):
        # main loop - iterating over item's children
        item, cookie = self.GetFirstChild(parent)
        while item:
           itemName = self.GetItemText(item)
           itemData = self.GetItemData(item)

           newItem = self.AppendItem(newParent, itemName)
           self.SetItemPyData(newItem, itemData)

           # if an item had children, we need to re-parent them as well
           if self.ItemHasChildren(item):
              # recursing...
              self.ReParent(item, newItem, )

           # continue iteration to the next child
           item, cookie = self.GetNextChild(parent, cookie)

    def ChangeHierarchy(self, itemName, x, y):
        parent = self.GetRootItem()
        item = self.traverse(parent, itemName)
        if item is None:
           return

        dragToItem, flags = self.HitTest(wx.Point(x, y))
        if dragToItem.IsOk():
           # prevent draging into itself
           if  dragToItem == item:
               return
           dragToItemName = self.GetItemText(dragToItem)
           if isinstance(self.paletteUI.palette.findItem(dragToItemName), ObjectBase):
              # this is a file node, bailing out
              return

           newItem = self.AppendItem(dragToItem, itemName)

           itemObj = self.paletteUI.palette.findItem(itemName)
           if itemObj is not None:
              # reparenting the data objects...
              if dragToItemName == self.rootName:
                 self.paletteUI.palette.add(itemObj)
              else:
                 self.paletteUI.palette.add(itemObj, dragToItemName)

           self.ReParent(item, newItem)
           self.Delete(item)

    def onBeginDrag(self, event):
        item = event.GetItem()

        if item != self.GetRootItem(): # prevent dragging root item
            text = self.GetItemText(item)
            print("Starting drag'n'drop with %s..." % repr(text))

            tdo = wx.TextDataObject(text)
            tds = wx.DropSource(self)
            tds.SetData(tdo)
            tds.DoDragDrop(True)

