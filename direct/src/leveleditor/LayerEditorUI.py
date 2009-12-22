"""
Defines Layer UI
"""
import wx
import sys
import cPickle as pickle
from pandac.PandaModules import *

import ObjectGlobals as OG

class LayerEditorUI(wx.Panel):
    def __init__(self, parent, editor):
        wx.Panel.__init__(self, parent)

        self.editor = editor
        self.editorTxt = "Layer Editor"
        self.layersDataDict = dict()
        self.llist = wx.ListCtrl(self, -1, style=wx.LC_REPORT|wx.LC_EDIT_LABELS|wx.LC_NO_HEADER)
        self.llist.InsertColumn(0, "Layers")

        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(self.llist, 1, wx.EXPAND, 0)
        self.SetSizer(sizer); self.Layout()

        parentSizer = wx.BoxSizer(wx.VERTICAL)
        parentSizer.Add(self, 1, wx.EXPAND, 0)
        parent.SetSizer(parentSizer); parent.Layout()

        self.opAdd       = "Add Layer"
        self.opDelete    = "Delete Layer"
        self.opAddObj    = "Add Selected Object"
        self.opRemoveObj = "Remove Selected Object"
        self.opShow      = "Show Members"
        self.opRename    = "Rename Layer"

        self.menuItems = list()
        self.menuItems.append(self.opAddObj)
        self.menuItems.append(self.opRemoveObj)
        self.menuItems.append(self.opShow)
        self.menuItems.append(self.opAdd)
        self.menuItems.append(self.opDelete)
        self.menuItems.append(self.opRename)

        self.popupmenu = wx.Menu()
        for item in self.menuItems:
            menuItem = self.popupmenu.Append(-1, item)
            self.Bind(wx.EVT_MENU, self.OnPopupItemSelected, menuItem)

        self.Bind(wx.EVT_CONTEXT_MENU, self.OnShowPopup)
        self.llist.Bind(wx.EVT_LIST_ITEM_ACTIVATED, self.onShowMembers)

    def OnShowPopup(self, event):
        #import pdb;set_trace()
        pos = event.GetPosition()
        pos = self.ScreenToClient(pos)
        self.PopupMenu(self.popupmenu, pos)

    def OnPopupItemSelected(self, event):
        menuItem = self.popupmenu.FindItemById(event.GetId())
        text = menuItem.GetText()
        if text == self.opAddObj:
           self.AddObj()
        elif text == self.opRemoveObj:
           self.RemoveObj()
        elif text == self.opShow:
           self.onShowMembers()
        elif text == self.opAdd:
           self.AddLayer()
        elif text == self.opDelete:
           self.DeleteLayer()
        elif text == self.opRename:
           self.RenameLayer()
        else:
           wx.MessageBox("You selected item '%s'" % text)

    def FindLabel(self, text):
        found = False
        for index in range(self.llist.GetItemCount()):
            itemtext = self.llist.GetItemText(index)
            if itemtext == text:
               return True
        return found

    def AddLayer(self):
        count = self.llist.GetItemCount()
        i = 1
        text = "Layer%s"%(count + i)
        found = self.FindLabel(text)
        while found:
              i = i + 1
              text = "Layer%s"%(count+i)
              found = self.FindLabel(text)
        index = self.llist.InsertStringItem(count, text)
        self.llist.SetItemData(index, count + i)
        layersData = list()
        self.layersDataDict[count + i] = layersData

    def DeleteLayer(self):
        import pdb;set_trace()
        index = self.llist.GetFirstSelected()
        if index != -1:
           key = self.llist.GetItemData(index)
           del(self.layersDataDict[key])
           item = self.llist.DeleteItem(index)

    def RenameLayer(self):
        #import pdb;set_trace()
        index = self.llist.GetFirstSelected()
        if index != -1:
           self.llist.SetItemState(index, wx.LIST_STATE_SELECTED, wx.LIST_STATE_SELECTED)
           self.llist.SetItemState(index, wx.LIST_STATE_FOCUSED, wx.LIST_STATE_FOCUSED)

    def RemoveObj(self):
        #import pdb;set_trace()
        objNodePath = base.direct.selected.last
        if objNodePath is None:
           wx.MessageBox("No object was selected.", self.editorTxt, wx.OK|wx.ICON_EXCLAMATION)
           return
        obj = self.editor.objectMgr.findObjectByNodePath(objNodePath)
        if obj is not None:
           layersDataDictKeys = self.layersDataDict.keys()
           for i in range(len(layersDataDictKeys)):
               layersData = self.layersDataDict[layersDataDictKeys[i]]
               for j in range(len(layersData)):
                   if layersData[j] == obj[OG.OBJ_UID]:
                      del(layersData[j])

    def AddObj(self):
        #import pdb;set_trace()
        index = self.llist.GetFirstSelected()
        if index == -1:
           wx.MessageBox("No layer was selected.", self.editorTxt,  wx.OK|wx.ICON_EXCLAMATION)
           return
        objNodePath = base.direct.selected.last
        if objNodePath is None:
           wx.MessageBox("No object was selected.", self.editorTxt, wx.OK|wx.ICON_EXCLAMATION)
           return

        # Checking if the object was laready added to the layer
        obj = self.editor.objectMgr.findObjectByNodePath(objNodePath)
        if obj is not None:
           i = self.llist.GetItemData(index)
           layersData = self.layersDataDict[i]
           for j in range(len(layersData)):
              if layersData[j] == obj[OG.OBJ_UID]:
                 wx.MessageBox("Selected object already is this layer", self.editorTxt, wx.OK|wx.ICON_EXCLAMATION)
                 return
           # Looking for the object in the other layers
           # If the object is found - delete it.
           self.RemoveObj()

           layersData.append(obj[OG.OBJ_UID])

    def onShowMembers(self, event):
        #import pdb;set_trace()
        item = event.GetItem()

        layerMembers = list()
        layerName = item.GetText()
        key = item.GetData()
        layerData = self.layersDataDict[key]
        for i in range(len(layerData)):
            obj = self.editor.objectMgr.findObjectById(layerData[i])
            namestr = "%s_%s"%(obj[OG.OBJ_DEF].name, obj[OG.OBJ_UID])
            layerMembers.append(namestr)
        dialog = wx.SingleChoiceDialog(None, layerName, self.editorTxt, layerMembers)
        if dialog.ShowModal() == wx.ID_OK:
           #do something here
           dialog.GetStringSelection()
        dialog.Destroy()

