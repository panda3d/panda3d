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

            #import pdb;set_trace()
            parent = self.editor.ui.protoPaletteUI.tree.GetSelection()
            if parent is None:
               parent = self.editor.ui.protoPaletteUI.root
            newItem = self.editor.ui.protoPaletteUI.tree.AppendItem(parent, name)
            self.editor.ui.protoPaletteUI.tree.SetItemPyData(newItem, itemData)
            self.editor.ui.protoPaletteUI.tree.ScrollTo(newItem)

class ProtoPaletteUITextDrop(wx.TextDropTarget):
    def __init__(self, editor):
        wx.TextDropTarget.__init__(self)
        self.editor = editor

    def OnDropText(self, x, y, text):
        self.editor.ui.protoPaletteUI.ChangeHierarchy(text, x, y)


class ProtoPaletteUI(wx.Panel):
    def __init__(self, parent, editor):
        wx.Panel.__init__(self, parent)

        self.editor = editor
        self.palette = self.editor.protoPalette
        #self.tree = wx.TreeCtrl(self)
        self.tree = wx.TreeCtrl(self, id=-1, pos=wx.DefaultPosition, 
                  size=wx.DefaultSize, style=wx.TR_EDIT_LABELS|wx.TR_DEFAULT_STYLE,
                  validator=wx.DefaultValidator, name="treeCtrl")
        self.rootName = "Proto Objects"
        self.root = self.tree.AddRoot(self.rootName)
        self.addTreeNodes(self.root, self.palette.data)

        self.opAdd    = "Add Group"
        self.opDelete = "Delete Group"

        self.menuItemsGen = list()
        self.menuItemsGen.append(self.opAdd)

        self.menuItemsSel = list()
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

        self.tree.Bind(wx.EVT_TREE_SEL_CHANGED, self.onSelected)
        self.tree.Bind(wx.EVT_TREE_BEGIN_DRAG, self.onBeginDrag)

        self.SetDropTarget(FileDrop(self.editor))
        self.SetDropTarget(ProtoPaletteUITextDrop(self.editor))

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
           if itemText != self.rootName:
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
           self.AddGroup()
        elif text == self.opDelete:
           self.DeleteGroup()

    def AddGroup(self):
        #import pdb;set_trace()
        parent = self.tree.GetSelection()
        if parent is None:
            parent = self.root

        i = 1
        namestr = "Group%s"%(i)
        found = self.Traverse(self.root, namestr)
        while found:
              i = i + 1
              namestr = "Group%s"%(i)
              found = self.Traverse(self.root, namestr)

        newItem = self.tree.AppendItem(parent, namestr)
        #uid = self.editor.objectMgr.genUniqueId()

        self.tree.Expand(self.root)

    def DeleteGroup(self):
        item = self.tree.GetSelection()
        itemText = self.tree.GetItemText(item)
        if item is not None and itemText != self.rootName:
           self.tree.Delete(item)

    def Traverse(self, parent, itemName):
        # prevent from traversing into self
        if itemName == self.tree.GetItemText(parent):
           return parent

        # main loop - serching for an item with an itemId
        item, cookie = self.tree.GetFirstChild(parent)
        while item:
              # if the item was found - return it
              if itemName == self.tree.GetItemText(item):
                 return item

              # the tem was not found - checking if it has children
              if self.tree.ItemHasChildren(item):
                 # item has children - delving into it
                 child = self.Traverse(item, itemName)
                 if child is not None:
                    return child

              # continue iteration to the next child
              item, cookie = self.tree.GetNextChild(parent, cookie)
        return None

    def ReParent(self, parent, newParent):

        # main loop - iterating over item's children
        item, cookie = self.tree.GetFirstChild(parent)
        while item:
           itemName = self.tree.GetItemText(item)
           itemData = self.tree.GetItemPyData(item)
           newItem = self.tree.AppendItem(newParent, itemName)
           self.tree.SetItemPyData(newItem, itemData)
           
           # if an item had children, we need to re-parent them as well
           if self.tree.ItemHasChildren(item):
              # recursing...
              self.ReParent(item, newItem)

           # continue iteration to the next child
           item, cookie = self.tree.GetNextChild(parent, cookie)

    def ChangeHierarchy(self, itemName, x, y):
        #import pdb;set_trace()
        parent = self.tree.GetRootItem()
        item = self.Traverse(parent, itemName)
        if item is None:
           return

        dragToItem, flags = self.tree.HitTest(wx.Point(x, y))
        if dragToItem.IsOk():
           # prevent draging into itself
           if  dragToItem == item:
               return
           newItem = self.tree.AppendItem(dragToItem, itemName)
           self.ReParent(item, newItem)
           self.tree.Delete(item)
