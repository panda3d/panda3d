"""
Defines Layer UI
"""
import wx
from panda3d.core import *

from . import ObjectGlobals as OG

class LayerEditorUI(wx.Panel):
    def __init__(self, parent, editor):
        wx.Panel.__init__(self, parent)

        self.editor = editor
        self.editorTxt = "Layer Editor"
        self.saveData = []
        self.layersDataDict = dict()
        self.layersDataDictNextKey = 0
        self.systemLayerKeys = []
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
        self.opRename    = "Rename Layer"
        self.opAddObj    = "Add Selected Object"
        self.opRemoveObj = "Remove Selected Object"
        self.opShowObj   = "Show Layer Objects"
        self.opHideObj   = "Hide Layer Objects"

        self.menuItemsGen = list()
        self.menuItemsGen.append(self.opAdd)
        #self.menuItems.append(self.opRename)

        self.menuItemsObj = list()
        self.menuItemsObj.append(self.opAddObj)
        self.menuItemsObj.append(self.opRemoveObj)
        self.menuItemsObj.append(self.opShowObj)
        self.menuItemsObj.append(self.opHideObj)
        self.menuItemsObj.append(self.opDelete)

        self.popupmenu = wx.Menu()
        for item in self.menuItemsGen:
            menuItem = self.popupmenu.Append(-1, item)
            self.Bind(wx.EVT_MENU, self.onPopupItemSelected, menuItem)

        self.Bind(wx.EVT_CONTEXT_MENU, self.onShowPopup)
        self.llist.Bind(wx.EVT_LIST_ITEM_ACTIVATED, self.onShowMembers)

    def menuAppendGenItems(self):
        for item in self.menuItemsGen:
            menuItem = self.popupmenu.Append(-1, item)
            self.Bind(wx.EVT_MENU, self.onPopupItemSelected, menuItem)

    def menuAppendObjItems(self, hitItem):
        for item in self.menuItemsObj:
            if hitItem in self.systemLayerKeys:
                if item in [self.opRemoveObj, self.opDelete, self.opAddObj]:
                    continue
            menuItem = self.popupmenu.Append(-1, item)
            self.Bind(wx.EVT_MENU, self.onPopupItemSelected, menuItem)

    def onShowPopup(self, event):
        pos = event.GetPosition()
        pos = self.ScreenToClient(pos)

        for menuItem in self.popupmenu.GetMenuItems():
            self.popupmenu.RemoveItem(menuItem)

        #import pdb;set_trace()
        hitItem, flags = self.llist.HitTest(pos)
        if hitItem == -1:
           self.menuAppendGenItems()
        else:
           self.menuAppendObjItems(hitItem)
        self.PopupMenu(self.popupmenu, pos)

    def onPopupItemSelected(self, event):
        menuItem = self.popupmenu.FindItemById(event.GetId())
        text = menuItem.GetText()
        if text == self.opAddObj:
           self.addObj()
        elif text == self.opRemoveObj:
           self.removeObj()
        elif text == self.opShowObj:
           self.HideObj(False)
        elif text == self.opHideObj:
           self.HideObj(True)
        elif text == self.opAdd:
           self.addLayer()
        elif text == self.opDelete:
           self.deleteLayer()
        elif text == self.opRename:
           self.renameLayer()
        else:
           wx.MessageBox("You selected item '%s'" % text)

    def reset(self):
        #import pdb;set_trace()
        self.layersDataDict.clear()
        self.layersDataDictNextKey = 0
        self.llist.DeleteAllItems()
        self.systemLayerKeys = []

    def findLabel(self, text):
        found = False
        for index in range(self.llist.GetItemCount()):
            itemtext = self.llist.GetItemText(index)
            if itemtext == text:
               return True
        return found

    def addLayerData(self, idx, objUID):
        self.removeObjData(objUID)
        layerData = self.layersDataDict[idx]
        layerData.append(objUID)

    def addLayerEntry(self, name, idx):
        index = self.llist.InsertStringItem(self.llist.GetItemCount(), name)
        self.llist.SetItemData(index, idx)
        layersData = list()
        self.layersDataDict[idx] = layersData
        if idx > self.layersDataDictNextKey:
           self.layersDataDictNextKey = idx

    def addLayer(self):
        #import pdb;set_trace()
        count = self.llist.GetItemCount()
        i = 1
        text = "Layer%s"%(count + i)
        found = self.findLabel(text)
        while found:
              i = i + 1
              text = "Layer%s"%(count + i)
              found = self.findLabel(text)

        self.layersDataDictNextKey = self.layersDataDictNextKey + 1
        self.addLayerEntry(text, self.layersDataDictNextKey)

    def deleteLayer(self):
        index = self.llist.GetFirstSelected()
        if index != -1:
           key = self.llist.GetItemData(index)
           del(self.layersDataDict[key])
           item = self.llist.DeleteItem(index)

    def renameLayer(self):
        index = self.llist.GetFirstSelected()
        if index != -1:
           self.llist.SetItemState(index, wx.LIST_STATE_SELECTED, wx.LIST_STATE_SELECTED)
           self.llist.SetItemState(index, wx.LIST_STATE_FOCUSED, wx.LIST_STATE_FOCUSED)

    def removeObjData(self, objUID):
        layersDataDictKeys = list(self.layersDataDict.keys())
        for i in range(len(layersDataDictKeys)):
            layersData = self.layersDataDict[layersDataDictKeys[i]]
            for j in range(len(layersData)):
                if layersData[j] == objUID:
                   del(layersData[j])

    def removeObj(self):
        objNodePath = base.direct.selected.last
        if objNodePath is None:
           wx.MessageBox("No object was selected.", self.editorTxt, wx.OK|wx.ICON_EXCLAMATION)
           return
        obj = self.editor.objectMgr.findObjectByNodePath(objNodePath)
        if obj is not None:
           self.removeObjData(obj[OG.OBJ_UID])

    def addObj(self):
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
           self.removeObj()

           layersData.append(obj[OG.OBJ_UID])

    def onShowMembers(self, event):
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

    def HideObj(self, hide):
        index = self.llist.GetFirstSelected()
        if index == -1:
           wx.MessageBox("No layer was selected.", self.editorTxt,  wx.OK|wx.ICON_EXCLAMATION)
           return

        key = self.llist.GetItemData(index)
        layerData = self.layersDataDict[key]
        if len(layerData) == 0:
           return
        for i in range(len(layerData)):
            obj = self.editor.objectMgr.findObjectById(layerData[i])
            if hide:
               obj[OG.OBJ_NP].hide()
            else:
               obj[OG.OBJ_NP].show()

        font = wx.Font
        font = self.llist.GetItemFont(index)
        if hide:
           font.SetWeight(wx.FONTWEIGHT_BOLD)
        else:
           font.SetWeight(wx.FONTWEIGHT_NORMAL)
        self.llist.SetItemFont(index, font)

    def traverse(self):
        self.saveData.append("\nif hasattr(base, 'le'):")
        self.saveData.append("    ui.layerEditorUI.reset()")
        for index in range(self.llist.GetItemCount()):
            self.saveData.append("    ui.layerEditorUI.addLayerEntry('%s', %s )"%(self.llist.GetItemText(index), self.llist.GetItemData(index)))
        layersDataDictKeys = list(self.layersDataDict.keys())
        for i in range(len(layersDataDictKeys)):
            layerData = self.layersDataDict[layersDataDictKeys[i]]
            for j in range(len(layerData)):
                self.saveData.append("    ui.layerEditorUI.addLayerData(%s, '%s')"%(layersDataDictKeys[i], layerData[j]))

    def getSaveData(self):
        self.saveData = []
        self.traverse()
        return self.saveData

