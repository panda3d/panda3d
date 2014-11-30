# Name:         XMLTreeMenu.py
# Purpose:      dynamic pulldown menu for XMLTree
# Author:       Roman Rolinsky <rolinsky@femagsoft.com>
# Created:      31.05.2007
# RCS-ID:       $Id: XMLTreeMenu.py 64107 2010-04-22 14:05:36Z ROL $

import wx
from globals import ID
from component import Manager

class XMLTreeMenu(wx.Menu):
    '''dynamic pulldown menu for XMLTree'''
    def __init__(self, container, comp, tree, createSibling, insertBefore):
        '''
        Create tree pull-down menu. createSibling flag is set if the
        child must be a sibling of the selected item, insertBefore
        flag is set if the child should be put before selected item in
        sibling mode or as the first child in non-sibling mode.
        '''
        wx.Menu.__init__(self)
        items = tree.GetSelections()
        if len(items) <= 1:
            item = tree.GetSelection()
            if not item: item = tree.root
            if not container:
                menu = self.CreateTopLevelMenu(comp)
            elif container is Manager.rootComponent and createSibling:
                menu = self.CreateTopLevelMenu(container)
            else:
                if createSibling:
                    menu = self.CreateSubMenus(container)
                else:
                    menu = self.CreateSubMenus(comp)
            # Select correct label for submenu
            if createSibling:
                if insertBefore:
                    self.AppendMenu(ID.SIBLING, 'Create Sibling', menu,
                                    'Create sibling before selected object')
                else:
                    self.AppendMenu(ID.SIBLING, 'Create Sibling', menu,
                                    'Create sibling after selected object')
            else:
                if insertBefore:
                    self.AppendMenu(ID.INSERT, 'Insert', menu,
                                    'Create object as the first child')
                else:
                    self.AppendMenu(ID.APPEND, 'Append', menu,
                                    'Create object as the last child')
            if comp is not Manager.rootComponent:
                self.Append(ID.SUBCLASS, 'Sublass...', 'Define subclass')
            self.AppendSeparator()
            
            if container:
                if container is Manager.rootComponent:
                    menu = self.CreateTopLevelMenu(container, ID.SHIFT)
                else:
                    menu = self.CreateSubMenus(container, ID.SHIFT)
                self.AppendMenu(ID.REPLACE, 'Replace With', menu,
                                'Replace selected object by another')
                self.AppendSeparator()
                
            self.Append(wx.ID_CUT, 'Cut', 'Cut to the clipboard')
            self.Append(wx.ID_COPY, 'Copy', 'Copy to the clipboard')
            if createSibling and item != tree.root:
                self.Append(ID.PASTE_SIBLING, 'Paste Sibling',
                            'Paste from the clipboard as a sibling')
            else:
                self.Append(ID.PASTE, 'Paste', 'Paste from the clipboard')
        if items:
            self.Append(wx.ID_DELETE, 'Delete', 'Delete selected objects')
        if comp.isContainer():
            self.AppendSeparator()
            self.Append(ID.EXPAND, 'Expand', 'Expand tree')
            self.Append(ID.COLLAPSE, 'Collapse', 'Collapse tree')

    def CreateTopLevelMenu(self, comp, idShift=0):
        m = wx.Menu()
        for index,component,label,help in Manager.menus['TOP_LEVEL']:
            if comp.canHaveChild(component):
                m.Append(component.id + idShift, label, help)
        m.AppendSeparator()
        m.Append(ID.REF, 'reference...', 'Create object_ref node')
        m.Append(ID.COMMENT, 'comment', 'Create comment node')        
        return m

    def CreateSubMenus(self, comp, idShift=0):
        menu = wx.Menu()
        for index,component,label,help in Manager.menus['ROOT']:
            if comp.canHaveChild(component):
                menu.Append(component.id + idShift, label, help)
        if menu.GetMenuItemCount():
            menu.AppendSeparator()
        for name in Manager.menuNames[2:]:
            # Skip empty menu groups
            if not Manager.menus.get(name, []): continue
            m = wx.Menu()
            for index,component,label,help in Manager.menus[name]:
                if comp.canHaveChild(component):
                    m.Append(component.id + idShift, label, help)
            if m.GetMenuItemCount():
                menu.AppendMenu(ID.MENU, name, m)
                menu.AppendSeparator()
            else:
                m.Destroy()
        menu.Append(ID.REF, 'reference...', 'Create object_ref node')
        menu.Append(ID.COMMENT, 'comment', 'Create comment node')
        return menu
