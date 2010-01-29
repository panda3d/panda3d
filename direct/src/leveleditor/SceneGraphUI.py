"""
Defines Scene Graph tree UI
"""
import wx
import cPickle as pickle
from pandac.PandaModules import *

import ObjectGlobals as OG

class SceneGraphUIDropTarget(wx.TextDropTarget):
    def __init__(self, editor):
        print "in SceneGraphUIDropTarget::init..."
        wx.TextDropTarget.__init__(self)
        self.editor = editor

    def OnDropText(self, x, y, text):
        print "in SceneGraphUIDropTarget::OnDropText..."
        self.editor.ui.sceneGraphUI.changeHierarchy(text, x, y)
        
class SceneGraphUI(wx.Panel):
    def __init__(self, parent, editor):
        wx.Panel.__init__(self, parent)

        self.editor = editor
        self.tree = wx.TreeCtrl(self)
        self.root = self.tree.AddRoot('render')
        self.tree.SetItemPyData(self.root, "render")

        self.shouldShowPandaObjChildren = False

        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(self.tree, 1, wx.EXPAND, 0)
        self.SetSizer(sizer); self.Layout()

        parentSizer = wx.BoxSizer(wx.VERTICAL)
        parentSizer.Add(self, 1, wx.EXPAND, 0)
        parent.SetSizer(parentSizer); parent.Layout()

        parent.SetDropTarget(SceneGraphUIDropTarget(self.editor))

        self.tree.Bind(wx.EVT_TREE_SEL_CHANGED, self.onSelected)
        self.tree.Bind(wx.EVT_TREE_BEGIN_DRAG, self.onBeginDrag)

    def reset(self):
        #import pdb;set_trace()
        itemList = list()
        item, cookie = self.tree.GetFirstChild(self.root)
        while item:
             itemList.append(item)
             item, cookie = self.tree.GetNextChild(self.root, cookie)

        for item in itemList:
            self.tree.Delete(item)

    def traversePandaObjects(self, parent, objNodePath):
        itemId = self.tree.GetItemPyData(parent)
        i = 0
        for child in objNodePath.getChildren():
            namestr = "%s.%s"%(child.node().getType(), child.node().getName())
            newItem = self.tree.PrependItem(parent, namestr)
            newItemId = "%s.%s"%(itemId, i)
            self.tree.SetItemPyData(newItem, newItemId)

            # recursing...
            self.traversePandaObjects(newItem, child)
            i = i + 1

    def addPandaObjectChildren(self, parent):
        # first, find Panda Object's NodePath of the item
        itemId = self.tree.GetItemPyData(parent)
        if itemId == "render":
           return
        obj = self.editor.objectMgr.findObjectById(itemId)
        if obj is None:
           return

        objNodePath = obj[OG.OBJ_NP]
        self.traversePandaObjects(parent, objNodePath)

        item, cookie = self.tree.GetFirstChild(parent)
        while item:
             # recursing...
             self.addPandaObjectChildren(item)
             item, cookie = self.tree.GetNextChild(parent, cookie)

    def removePandaObjectChildren(self, parent):
        # first, find Panda Object's NodePath of the item
        itemId = self.tree.GetItemPyData(parent)
        if itemId == "render":
           return
        obj = self.editor.objectMgr.findObjectById(itemId)
        if obj is None:
           self.tree.Delete(parent)
           return
        item, cookie = self.tree.GetFirstChild(parent)
        while item:
           # recurse...
           itemToRemove = item
           # continue iteration to the next child
           item, cookie = self.tree.GetNextChild(parent, cookie)
           self.removePandaObjectChildren(itemToRemove)

    def getTreeItem(self, parent, itemId):
        item, cookie = self.tree.GetFirstChild(parent)
        while item:
             if itemId == self.tree.GetItemPyData(item):
                return item
             child = None
             if self.tree.ItemHasChildren(item):
                child = self.getTreeItem(item, itemId)
             if child is not None:
                return child
             # continue iteration to the next child
             item, cookie = self.tree.GetNextChild(parent, cookie)
        return None

    def add(self, item):
        #import pdb;pdb.set_trace()
        if item is None:
           return
        obj = self.editor.objectMgr.findObjectByNodePath(NodePath(item))
        if obj is None:
           return

        parentNodePath = obj[OG.OBJ_NP].getParent()
        parentObj = self.editor.objectMgr.findObjectByNodePath(parentNodePath)

        #import pdb;pdb.set_trace()
        if parentObj is None:
            parent = self.root
        else:
            parent = self.getTreeItem(self.root, parentObj[OG.OBJ_UID])

        namestr = "%s_%s"%(obj[OG.OBJ_DEF].name, obj[OG.OBJ_UID])
        newItem = self.tree.AppendItem(parent, namestr)
        self.tree.SetItemPyData(newItem, obj[OG.OBJ_UID])
        
        # adding children of PandaObj
        if self.shouldShowPandaObjChildren:
           self.addPandaObjectChildren(newItem)
        self.tree.Expand(self.root)

    def traverse(self, parent, itemId):
        print "in traverse for itemId=%s..." %repr(itemId)

        # prevent from traversing into self
        if itemId == self.tree.GetItemPyData(parent):
           return None

        # main loop - serching for an item with an itemId
        item, cookie = self.tree.GetFirstChild(parent)
        while item:
              # if the item was found - return it
              if itemId == self.tree.GetItemPyData(item):
                 return item

              # the tem was not found - checking if it has children
              if self.tree.ItemHasChildren(item):
                 # item has children - delving into it
                 child = self.traverse(item, itemId)
                 if child is not None:
                    return child
                    
              # continue iteration to the next child
              item, cookie = self.tree.GetNextChild(parent, cookie)
        return None

    def reParent(self, parent, newParent):
        #import pdb;set_trace()

        # main loop - iterating over item's children
        item, cookie = self.tree.GetFirstChild(parent)
        while item:
           data = self.tree.GetItemText(item)
           itemId = self.tree.GetItemPyData(item)
           newItem = self.tree.AppendItem(newParent, data)
           self.tree.SetItemPyData(newItem, itemId)
           
           # if an item had children, we need to re-parent them as well
           if self.tree.ItemHasChildren(item):
              # recursing...
              self.reParent(item, newItem)

           # continue iteration to the next child
           item, cookie = self.tree.GetNextChild(parent, cookie)

    def changeHierarchy(self, data, x, y):
        itemText = data.split('_')
        itemId = itemText[1]
        parent = self.tree.GetRootItem()
        item = self.traverse(parent, itemId)
        if item is None:
           return

        dragToItem, flags = self.tree.HitTest(wx.Point(x, y))
        if dragToItem.IsOk():
           # prevent draging into itself
           if  dragToItem == item:
               return
           newItem = self.tree.AppendItem(dragToItem, data)
           self.tree.SetItemPyData(newItem, itemId)
           self.reParent(item, newItem)
           oldParent = self.tree.GetItemParent(item)
           self.tree.Delete(item)

           obj = self.editor.objectMgr.findObjectById(itemId)
           itemId = self.tree.GetItemPyData(dragToItem)
           dragToItemObj = None
           if itemId != "render":
              dragToItemObj = self.editor.objectMgr.findObjectById(itemId)

           objNodePath = obj[OG.OBJ_NP]
           if dragToItemObj is None:
              objNodePath.wrtReparentTo(render)
           else:
              objNodePath.wrtReparentTo(dragToItemObj[OG.OBJ_NP])

           if self.shouldShowPandaObjChildren:
              self.removePandaObjectChildren(oldParent)
              self.addPandaObjectChildren(oldParent)
              self.removePandaObjectChildren(dragToItem)
              self.addPandaObjectChildren(dragToItem)


    def showPandaObjectChildren(self):
        itemList = list()
        self.shouldShowPandaObjChildren = not self.shouldShowPandaObjChildren

        item, cookie = self.tree.GetFirstChild(self.root)
        while item:
             itemList.append(item)
             item, cookie = self.tree.GetNextChild(self.root, cookie)

        #import pdb;set_trace()
        for item in itemList:
             if self.shouldShowPandaObjChildren:
                self.addPandaObjectChildren(item)
             else:
                self.removePandaObjectChildren(item)
             # continue iteration to the next child

    def onSelected(self, event):
        itemId = self.tree.GetItemPyData(event.GetItem())
        if itemId:
            obj = self.editor.objectMgr.findObjectById(itemId);
            if obj:
                base.direct.select(obj[OG.OBJ_NP])

    def onBeginDrag(self, event):
        item = event.GetItem()

        if item != self.tree.GetRootItem(): # prevent dragging root item
            text = self.tree.GetItemText(item)
            print "Starting SceneGraphUI drag'n'drop with %s..." % repr(text)

            tdo = wx.TextDataObject(text)
            tds = wx.DropSource(self.tree)
            tds.SetData(tdo)
            tds.DoDragDrop(True)
