# Name:         undo.py
# Purpose:      XRC editor, undo/redo module
# Author:       Roman Rolinsky <rolinsky@mema.ucl.ac.be>
# Created:      01.12.2002
# RCS-ID:       $Id: undo.py 54812 2008-07-29 13:39:00Z ROL $

from globals import *
import view
from component import Manager
from model import Model

undo_depth = 10                 # max number of undo remembered

# Undo/redo classes
class UndoManager:
    # Undo/redo stacks
    undo = []
    redo = []
    def RegisterUndo(self, undoObj):
        TRACE('RegisterUndo: %s', undoObj.label)
        self.undo.append(undoObj)
        while len(self.undo) > undo_depth: self.undo.pop(0)
        map(Undo.destroy, self.redo)
        self.redo = []
        self.UpdateToolHelp()
    def GetUndoLabel(self):
        return self.undo[-1].label
    def GetRedoLabel(self):
        return self.redo[-1].label
    def Undo(self):
        undoObj = self.undo.pop()
        undoObj.undo()
        self.redo.append(undoObj)
        view.frame.SetStatusText('Undone')
        Presenter.setModified()
        self.UpdateToolHelp()
    def Redo(self):
        undoObj = self.redo.pop()
        undoObj.redo()
        self.undo.append(undoObj)
        view.frame.SetStatusText('Redone')
        Presenter.setModified()
        self.UpdateToolHelp()
    def Clear(self):
        map(Undo.destroy, self.undo)
        self.undo = []
        map(Undo.destroy, self.redo)
        self.redo = []
        self.UpdateToolHelp()
    def CanUndo(self):
        return bool(self.undo)
    def CanRedo(self):
        return bool(self.redo)
    def UpdateToolHelp(self):
        if g.undoMan.CanUndo(): 
            msg = 'Undo ' + self.GetUndoLabel()
            view.frame.tb.SetToolShortHelp(wx.ID_UNDO, msg)
            view.frame.tb.SetToolLongHelp(wx.ID_UNDO, msg)
        if g.undoMan.CanRedo(): 
            msg = 'Redo ' + self.GetRedoLabel()
            view.frame.tb.SetToolShortHelp(wx.ID_REDO, msg)
            view.frame.tb.SetToolLongHelp(wx.ID_REDO, msg)

class Undo:
    '''ABC for Undo*.'''
    def redo(self):             # usually redo is same as undo
        self.undo()
    def destroy(self):
        pass

class UndoCutDelete(Undo):
    label = 'cut/delete'
    def __init__(self, itemIndex, state, node):
        self.itemIndex = itemIndex
        self.state = state
        self.node = node
    def destroy(self):
        if self.node: self.node.unlink()
        self.node = None
    def undo(self):
        Presenter.unselect()
        # Updating DOM. Find parent node first
        parentItem = view.tree.ItemAtFullIndex(self.itemIndex[:-1])
        parentNode = view.tree.GetPyData(parentItem)
        parentComp = Manager.getNodeComp(parentNode)
        nextItem = view.tree.ItemAtFullIndex(self.itemIndex)
        if nextItem:
            nextNode = parentComp.getTreeOrImplicitNode(view.tree.GetPyData(nextItem))
        else:
            nextNode = None
        # Insert before next
        parentNode.insertBefore(self.node, nextNode)
        # Remember test window item
        if view.testWin.item is not None:
            testItemIndex = view.tree.ItemFullIndex(view.testWin.item)
        # Update tree and presenter
        view.tree.FlushSubtree(parentItem, parentNode)
        view.tree.SetFullState(self.state)
        # Restore test window item
        if view.testWin.item is not None:
            view.testWin.item = view.tree.ItemAtFullIndex(testItemIndex)
        item = view.tree.ItemAtFullIndex(self.itemIndex)
        view.tree.EnsureVisible(item)
        # This will generate events
        view.tree.SelectItem(item)
    def redo(self):
        item = view.tree.ItemAtFullIndex(self.itemIndex)
        Presenter.setData(item)
        self.node = Presenter.delete(item)

# Undoing paste/create is the opposite of cut/delete, so we can reuse
# UndoCutDelete class swapping undo<->redo
class UndoPasteCreate(UndoCutDelete):
    label = 'paste/create'
    # The ctor is different because node is not known initially
    def __init__(self, itemIndex, state):
        self.itemIndex = itemIndex # new item index
        self.state = state                 # tree state
        self.node = None
    undo = UndoCutDelete.redo
    redo = UndoCutDelete.undo

class UndoReplace(Undo):
    label = 'replace'
    def __init__(self, itemIndex, comp, node):
        self.itemIndex = itemIndex
        self.comp = comp
        self.node = node
    def destroy(self):
        if self.node: self.node.unlink()
        self.node = None
    def undo(self):
        # Replace current node with old node
        Presenter.unselect()
        item = view.tree.ItemAtFullIndex(self.itemIndex)
        Presenter.setData(item)
        comp = self.comp
        node = self.node
        data = wx.TreeItemData(node)
        parentItem = view.tree.GetItemParent(item)
        parentNode = view.tree.GetPyData(parentItem)
        self.node = view.tree.GetPyData(item)
        self.comp = Presenter.comp
        Presenter.container.replaceChild(parentNode, node, self.node)
        # Replace tree item: insert new, remove old
        label = comp.getTreeText(node)
        imageId = comp.getTreeImageId(node)
        item = view.tree.InsertItem(parentItem, item, label, imageId, data=data)
        view.tree.Delete(view.tree.GetPrevSibling(item))
        Presenter.item = item
        # Add children
        for n in filter(is_object, node.childNodes):
            view.tree.AddNode(item, comp.getTreeNode(n))
        view.tree.EnsureVisible(item)
        # Update panel
        view.tree.SelectItem(item)
        Presenter.setModified()        

class UndoEdit(Undo):
    '''Undo class for using in AttributePanel.'''
    label = 'edit'
    def __init__(self, item, page):
        self.index = view.tree.ItemFullIndex(item)
        self.page = page
        panel = view.panel.nb.GetPage(page).panel
        self.values = panel.GetValues()
    def undo(self):
        # Go back to the previous item
        Presenter.unselect()
        item = view.tree.ItemAtFullIndex(self.index)
        Presenter.setData(item)
        panel = view.panel.nb.GetPage(self.page).panel
        values = panel.GetValues()
        panel.SetValues(self.values)
        Presenter.update(item)
        self.values = values
        view.tree.SelectItem(item)

class UndoGlobal(Undo):
    '''Undo class storing a copy of the complete tree. Can be used for
    non-frequent operations to avoid programming special undo
    classes.'''
    label = 'global'
    def __init__(self):
        self.mainNode = Model.mainNode.cloneNode(True)
        self.state = view.tree.GetFullState()
    def destroy(self):
        self.mainNode.unlink()
    def undo(self):
        # Exchange
        Model.mainNode,self.mainNode = \
            self.mainNode,Model.dom.replaceChild(self.mainNode, Model.mainNode)
        # Replace testElem
        Model.testElem = Model.mainNode.childNodes[0]
        state = view.tree.GetFullState()
        Presenter.flushSubtree()
        view.tree.SetFullState(self.state)
        self.state = state
    def redo(self):
        self.undo()
        Presenter.unselect()
