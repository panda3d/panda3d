# Name:         gizmos.py
# Purpose:      XML handlers for wx.gismos classes
# Author:       Roman Rolinsky <rolinsky@femagsoft.com>
# Created:      09.07.2007
# RCS-ID:       $Id$

import wx
import wx.xrc as xrc
import wx.gizmos as gizmos

class LEDNumberCtrlXmlHandler(xrc.XmlResourceHandler):
    def __init__(self):
        xrc.XmlResourceHandler.__init__(self)
        # Standard styles
        self.AddWindowStyles()
        # Custom styles
        self.AddStyle('wxLED_ALIGN_LEFT', gizmos.LED_ALIGN_LEFT)
        self.AddStyle('wxLED_ALIGN_RIGHT', gizmos.LED_ALIGN_RIGHT)
        self.AddStyle('wxLED_ALIGN_CENTER', gizmos.LED_ALIGN_CENTER)
        self.AddStyle('wxLED_DRAW_FADED', gizmos.LED_DRAW_FADED)
        
    def CanHandle(self,node):
        return self.IsOfClass(node, 'LEDNumberCtrl')

    # Process XML parameters and create the object
    def DoCreateResource(self):
        assert self.GetInstance() is None
        w = gizmos.LEDNumberCtrl(self.GetParentAsWindow(),
                                 self.GetID(),
                                 self.GetPosition(),
                                 self.GetSize(),
                                 self.GetStyle())
        # wxLED_ALIGN_MASK was incorrect
        align = self.GetStyle() & 7
        if align: w.SetAlignment(self.GetStyle() & 7)
        w.SetValue(self.GetText('value'))
        self.SetupWindow(w)
        return w


class EditableListBoxXmlHandler(xrc.XmlResourceHandler):
    def __init__(self):
        xrc.XmlResourceHandler.__init__(self)
        # Standard styles
        self.AddWindowStyles()
        # Custom styles
        self.AddStyle('wxEL_ALLOW_NEW', gizmos.EL_ALLOW_NEW)
        self.AddStyle('wxEL_ALLOW_EDIT', gizmos.EL_ALLOW_EDIT)
        self.AddStyle('wxEL_ALLOW_DELETE', gizmos.EL_ALLOW_DELETE)
        
    def CanHandle(self, node):
        return self.IsOfClass(node, 'EditableListBox')
#        return self.IsOfClass(node, 'EditableListBox') or \
#               self.insideBox and node.GetName() == 'item'

    # Process XML parameters and create the object
    def DoCreateResource(self):
        assert self.GetInstance() is None
        
        w = gizmos.EditableListBox(self.GetParentAsWindow(),
                                   self.GetID(),
                                   self.GetText("label"),
                                   self.GetPosition(),
                                   self.GetSize(),
                                   self.GetStyle(),
                                   self.GetName())
        
        # Doesn't work
        #self.insideBox = True
        #self.CreateChildrenPrivately(None, self.GetParamNode('content'))
        #self.insideBox = False
        
        # Long way
        strings = []
        n = self.GetParamNode('content')
        if n: n = n.GetChildren()
        while n:
            if n.GetType() != xrc.XML_ELEMENT_NODE or n.GetName() != "item":
                n = n.GetNext()
                continue
            strings.append(n.GetNodeContent())
            n = n.GetNext()
        w.SetStrings(strings)
        self.SetupWindow(w)
        return w


class TreeListCtrlXmlHandler(xrc.XmlResourceHandler):
    def __init__(self):
        xrc.XmlResourceHandler.__init__(self)
        # Standard styles
        self.AddWindowStyles()
        # Custom styles
        self.AddStyle('wxDEFAULT_COL_WIDTH', gizmos.DEFAULT_COL_WIDTH)
        self.AddStyle('wxTL_MODE_NAV_FULLTREE', gizmos.TL_MODE_NAV_FULLTREE)
        self.AddStyle('wxTL_MODE_NAV_EXPANDED', gizmos.TL_MODE_NAV_EXPANDED)
        self.AddStyle('wxTL_MODE_NAV_VISIBLE', gizmos.TL_MODE_NAV_VISIBLE)
        self.AddStyle('wxTL_MODE_NAV_LEVEL', gizmos.TL_MODE_NAV_LEVEL)
        self.AddStyle('wxTL_MODE_FIND_EXACT', gizmos.TL_MODE_FIND_EXACT)
        self.AddStyle('wxTL_MODE_FIND_PARTIAL', gizmos.TL_MODE_FIND_PARTIAL)
        self.AddStyle('wxTL_MODE_FIND_NOCASE', gizmos.TL_MODE_FIND_NOCASE)
        self.AddStyle('wxTREE_HITTEST_ONITEMCOLUMN', gizmos.TREE_HITTEST_ONITEMCOLUMN)
        self.AddStyle('wxTR_COLUMN_LINES', gizmos.TR_COLUMN_LINES)
        self.AddStyle('wxTR_VIRTUAL', gizmos.TR_VIRTUAL)
        self.AddStyle('wxTL_ALIGN_LEFT  ', wx.ALIGN_LEFT)
        self.AddStyle('wxTL_ALIGN_RIGHT ', wx.ALIGN_RIGHT)
        self.AddStyle('wxTL_ALIGN_CENTER', wx.ALIGN_CENTER)
        
        self.AddStyle('wxTL_SEARCH_VISIBLE', gizmos.TL_MODE_NAV_VISIBLE)
        self.AddStyle('wxTL_SEARCH_LEVEL  ', gizmos.TL_MODE_NAV_LEVEL)
        self.AddStyle('wxTL_SEARCH_FULL   ', gizmos.TL_MODE_FIND_EXACT)
        self.AddStyle('wxTL_SEARCH_PARTIAL', gizmos.TL_MODE_FIND_PARTIAL)
        self.AddStyle('wxTL_SEARCH_NOCASE ', gizmos.TL_MODE_FIND_NOCASE)

        self.AddStyle('wxTR_DONT_ADJUST_MAC', gizmos.TR_DONT_ADJUST_MAC)
        self.AddStyle('wxTR_DEFAULT_STYLE', wx.TR_DEFAULT_STYLE)
        
    def CanHandle(self, node):
        return self.IsOfClass(node, 'TreeListCtrl')

    # Process XML parameters and create the object
    def DoCreateResource(self):
        assert self.GetInstance() is None
        
        w = gizmos.TreeListCtrl(self.GetParentAsWindow(),
                                self.GetID(),
                                style=self.GetStyle(),
                                name=self.GetName())
        w.AddColumn("Main column")
        w.AddColumn('Column 1')
        w.SetMainColumn(0)
        w.SetColumnWidth(0, 50)
        w.SetColumnWidth(1, 50)
        root = w.AddRoot('Root')
        w.SetItemText(root, "col 1", 1)
        item1 = w.AppendItem(root, 'item 1')
        w.SetItemText(item1, "col 1", 1)
        w.Expand(root)
        return w


class DynamicSashWindowXmlHandler(xrc.XmlResourceHandler):
    def __init__(self):
        xrc.XmlResourceHandler.__init__(self)
        # Standard styles
        self.AddWindowStyles()
        # Custom styles
        self.AddStyle('wxDS_MANAGE_SCROLLBARS', gizmos.DS_MANAGE_SCROLLBARS)
        self.AddStyle('wxDS_DRAG_CORNER', gizmos.DS_DRAG_CORNER)
        
    def CanHandle(self, node):
        return self.IsOfClass(node, 'DynamicSashWindow')

    # Process XML parameters and create the object
    def DoCreateResource(self):
        assert self.GetInstance() is None
        
        w = gizmos.DynamicSashWindow(self.GetParentAsWindow(),
                                     self.GetID(),
                                     self.GetPosition(),
                                     self.GetSize(),
                                     self.GetStyle(),
                                     self.GetName())
        
        self.SetupWindow(w)
        return w

