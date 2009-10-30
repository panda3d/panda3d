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

        # self.palette = self.editor.objectPalette
        self.tree = wx.TreeCtrl(self)
        self.root = self.tree.AddRoot('render')
        #self.addTreeNodes(root, self.palette.data)

        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(self.tree, 1, wx.EXPAND, 0)
        self.SetSizer(sizer); self.Layout()

        parentSizer = wx.BoxSizer(wx.VERTICAL)
        parentSizer.Add(self, 1, wx.EXPAND, 0)
        parent.SetSizer(parentSizer); parent.Layout()

        parent.SetDropTarget(SceneGraphUIDropTarget(self.editor))

        self.tree.Bind(wx.EVT_TREE_SEL_CHANGED, self.onSelected)
        self.tree.Bind(wx.EVT_TREE_BEGIN_DRAG, self.onBeginDrag)

    def add(self, item, parentName = None):
        # import pdb;pdb.set_trace()
        if item is None:
           return
        obj = self.editor.objectMgr.findObjectByNodePath(NodePath(item))
        if obj is None:
           print "During adding to SceneGraphUI couldn't find obj..."
           return

        namestr = "%s_%s"%(obj[OG.OBJ_DEF].name, obj[OG.OBJ_UID])
        print "Adding as result of a drag'n'drop %s..." % repr(namestr)
        newItem = self.tree.AppendItem(self.root, namestr)
        self.tree.SetItemPyData(newItem, obj[OG.OBJ_UID])

    def traverse(self, parent, itemId):
        print "in traverse for itemId=%s..." %repr(itemId)

        # prevent from traversing into self
        if itemId == self.tree.GetItemPyData(parent):
           return None

        # main loop - serching for an item with an itemId
        item, cookie = self.tree.GetFirstChild(parent)
        while item:
              print "in traverse, looping - if %s==%s" %(itemId, self.tree.GetItemPyData(item))
              
              # if the item was found - return it
              if itemId == self.tree.GetItemPyData(item):
                 print "traverse found %s..." % repr(self.tree.GetItemText(item))
                 return item
                 
              # the tem was not found - checking if it has children
              if self.tree.ItemHasChildren(item):
                 print "in traverse, item=%s has a child" %repr(self.tree.GetItemPyData(item))
                 
                 # item has children - delving into it
                 child = self.traverse(item, itemId)
                 if child is not None:
                    return child
                    
              # continue iteration to the next child
              item, cookie = self.tree.GetNextChild(parent, cookie)

        print "traverse didn't find anything..."
        return None

    def reParent(self, parent, newParent):
        #import pdb;set_trace()
        print "in reParent for %s..." %repr(self.tree.GetItemText(parent))

        # main loop - iterating over item's children
        item, cookie = self.tree.GetFirstChild(parent)
        while item:
           data = self.tree.GetItemText(item)
           itemId = self.tree.GetItemPyData(item)
           newItem = self.tree.AppendItem(newParent, data)
           self.tree.SetItemPyData(newItem, itemId)

           # if an item had children, we need to re-parent them as well
           if self.tree.ItemHasChildren(item):
              print "in reParent item=%s has children..." %repr(self.tree.GetItemText(item))
              self.reParent(item, newItem)

           # continue iteration to the next child
           item, cookie = self.tree.GetNextChild(parent, cookie)

    def changeHierarchy(self, data, x, y):
        print "Changing hierarchy for %s..." % repr(data)

        itemText = data.split('_')
        itemId = itemText[1]
        parent = self.tree.GetRootItem()
        item = self.traverse(parent, itemId)
        if item is None:
           return

        dragToItem, flags = self.tree.HitTest(wx.Point(x, y))
        print "dragToItem=%s" %repr(dragToItem)
        if dragToItem.IsOk():
           # prevent draging into itself
           if  dragToItem == item:
               return
           print "About to AppendItem %s" %data
           newItem = self.tree.AppendItem(dragToItem, data)
           self.tree.SetItemPyData(newItem, itemId)

           obj = self.editor.objectMgr.findObjectById(itemId)
           dragToItemObj = self.editor.objectMgr.findObjectById(self.tree.GetItemPyData(dragToItem))

           objNodePath = obj[OG.OBJ_NP]
           dragToItemObjNodePath = dragToItemObj[OG.OBJ_NP]
           objNodePath.wrtReparentTo(dragToItemObjNodePath)

           self.reParent(item, newItem)
           self.tree.Delete(item)

    def onSelected(self, event):
        itemId = self.tree.GetItemPyData(event.GetItem())
        if itemId:
            obj = self.editor.objectMgr.findObjectById(itemId);
            self.editor.objectMgr.selectObject(obj[OG.OBJ_NP])

    def onBeginDrag(self, event):
        item = event.GetItem()

        if item != self.tree.GetRootItem(): # prevent dragging root item
            text = self.tree.GetItemText(item)
            print "Starting SceneGraphUI drag'n'drop with %s..." % repr(text)

            tdo = wx.TextDataObject(text)
            tds = wx.DropSource(self.tree)
            tds.SetData(tdo)
            tds.DoDragDrop(True)
