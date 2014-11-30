# Name:         AttributePanel.py
# Purpose:      View components for editing attributes
# Author:       Roman Rolinsky <rolinsky@mema.ucl.ac.be>
# Created:      17.06.2007
# RCS-ID:       $Id: AttributePanel.py 65408 2010-08-25 22:52:11Z RD $

import string
import wx
import wx.lib.buttons as buttons
from globals import *
import params
import component
import undo
import images


labelSize = (100,-1)

# Panel class is the attribute panel containing class name, XRC ID and
# a notebook with particular pages.

class ScrolledPage(wx.ScrolledWindow):
    def __init__(self, parent):
        wx.ScrolledWindow.__init__(self, parent)
        self.topSizer = wx.BoxSizer(wx.VERTICAL)
        self.SetSizer(self.topSizer)
        self.panel = None
        self.SetScrollRate(5, 5)

    def Reset(self):
        if self.panel:
            self.panel.Destroy()
            self.panel = None

    def SetPanel(self, panel):
        self.Reset()
        self.panel = panel
        self.topSizer.Add(panel, 0, wx.ALL | wx.EXPAND, 2)
        self.topSizer.Layout()
        self.SendSizeEvent()

# No highlighting please
class ToggleButton(buttons.GenBitmapToggleButton):
    def GetBackgroundBrush(self, dc):
        colBg = self.GetBackgroundColour()
        brush = wx.Brush(colBg, wx.SOLID)
        if self.style & wx.BORDER_NONE:
            myAttr = self.GetDefaultAttributes()
            parAttr = self.GetParent().GetDefaultAttributes()
            myDef = colBg == myAttr.colBg
            parDef = self.GetParent().GetBackgroundColour() == parAttr.colBg
            if myDef and parDef:
                if wx.Platform == "__WXMAC__":
                    brush.MacSetTheme(1) # 1 == kThemeBrushDialogBackgroundActive
                elif wx.Platform == "__WXMSW__":
                    if self.DoEraseBackground(dc):
                        brush = None
            elif myDef and not parDef:
                colBg = self.GetParent().GetBackgroundColour()
                brush = wx.Brush(colBg, wx.SOLID)
        return brush

class Panel(wx.Panel):
    '''Attribute panel main class.'''
    def __init__(self, *args, **kw):
        wx.Panel.__init__(self, *args, **kw)

        # Set common sizes
        params.InitParams(self)

        topSizer = wx.BoxSizer(wx.VERTICAL)
        pinSizer = wx.BoxSizer(wx.HORIZONTAL)
        sizer = wx.FlexGridSizer(cols=2, vgap=1, hgap=5)
        self.labelRef = wx.StaticText(self, -1, 'ref:')
        self.textRef = params.ParamText(self, 'ref', textWidth=200)
        sizer.AddMany([ (self.labelRef, 0, wx.ALIGN_CENTER_VERTICAL),
                        (self.textRef, 0, wx.LEFT, 5) ])
        self.labelClass = wx.StaticText(self, -1, 'class:')
        self.textClass = params.ParamText(self, 'class', textWidth=200)
        sizer.AddMany([ (self.labelClass, 0, wx.ALIGN_CENTER_VERTICAL),
                        (self.textClass, 0, wx.LEFT, 5) ])
        self.labelName = wx.StaticText(self, -1, 'name:')
        self.textName = params.ParamText(self, 'name', textWidth=200)
        sizer.AddMany([ (self.labelName, 0, wx.ALIGN_CENTER_VERTICAL),
                        (self.textName, 0, wx.LEFT, 5) ])
        pinSizer.Add(sizer, 0, wx.ALL, 5)
        pinSizer.Add((0, 0), 1)
        self.pinButton = ToggleButton(self, bitmap=images.ToolPin.GetBitmap(),
                                      style=wx.BORDER_NONE)
        self.pinButton.SetBitmapSelected(images.ToolPinDown.GetBitmap())
        self.pinButton.SetToggle(g.conf.panelPinState)
        self.pinButton.SetToolTipString('Sticky page selection')
        pinSizer.Add(self.pinButton)
        topSizer.Add(pinSizer, 0, wx.EXPAND)
        self.sizer = sizer

        self.nb = wx.Notebook(self, -1)
        if wx.Platform == '__WXGTK__':
            # Redefine AddPage on GTK to fix when page added is not shown
            _oldAddPage = wx.Notebook.AddPage
            def _newAddPage(self, page, label):
                _oldAddPage(self, page, label)
                page.Show(True)
            wx.Notebook.AddPage = _newAddPage

        # Create scrolled windows for panels
        self.pageA = ScrolledPage(self.nb)
        self.nb.AddPage(self.pageA, 'Attributes')
        # Style page
        self.pageStyle = ScrolledPage(self.nb)
        self.pageStyle.Hide()
        # Extra style page
        self.pageExStyle = ScrolledPage(self.nb)
        self.pageExStyle.Hide()
        # Window attributes page
        self.pageWA = ScrolledPage(self.nb)
        self.pageWA.Hide()
        # Implicit attributes page
        self.pageIA = ScrolledPage(self.nb)
        self.pageIA.Hide()
        # Code page
        self.pageCode = ScrolledPage(self.nb)
        self.pageCode.Hide()

        topSizer.Add(self.nb, 1, wx.EXPAND)
        self.SetSizer(topSizer)

        self.undo = None        # pending undo object

    def SetData(self, container, comp, node):
        self.Freeze()

        oldLabel = self.nb.GetPageText(self.nb.GetSelection())
        self.nb.SetSelection(0)
        map(self.nb.RemovePage, range(self.nb.GetPageCount()-1, 0, -1))
        
        self.container = container
        self.comp = comp
        self.node = node        # main node
        self.refNode = node
        panels = []
        # Class and name
        if node.nodeType == node.COMMENT_NODE:
            self.labelRef.Hide()
            self.textRef.Hide()
            self.labelClass.Hide()
            self.textClass.Hide()            
        elif node.tagName == 'object_ref':
            self.labelRef.Show()
            self.textRef.Show()
            self.textRef.SetValue(node.getAttribute('ref'))
            self.labelClass.Hide()
            self.textClass.Hide()
            # 'class' can be present for ref?
            self.textClass.SetValue(node.getAttribute('class'))
        else:
            self.labelRef.Hide()
            self.textRef.Hide()
            if comp.klass != 'root':
                self.labelClass.Show()
                self.textClass.Show()
                subclass = node.getAttribute('subclass')
                if not subclass:
                    self.textClass.SetValue(node.getAttribute('class'))
                else:
                    self.textClass.SetValue(subclass + '(%s)' % node.getAttribute('class'))
            else:                   # root node
                self.labelClass.Hide()
                self.textClass.Hide()
                self.labelRef.Hide()
                self.textRef.Hide()
        self.labelName.Show(comp.hasName)
        self.textName.Show(comp.hasName)
        if comp.hasName:
            self.textName.SetValue(node.getAttribute('name'))

        self.Layout()           # update after hiding/showing

        panel = AttributePanel(self.pageA, comp.attributes, comp.params, comp.renameDict)
        panels.append(panel)
        self.pageA.SetPanel(panel)
        self.SetValues(panel, node)

        if comp.windowAttributes:
            panel = AttributePanel(self.pageWA, comp.windowAttributes,
                                   rename_dict = params.WARenameDict)
            panels.append(panel)
            self.pageWA.SetPanel(panel)
            self.nb.AddPage(self.pageWA, "Look'n'Feel")
            self.SetValues(panel, node)

        if comp.styles or comp.genericStyles:
            # Create style page
            panel = params.StylePanel(self.pageStyle, comp.styles, comp.genericStyles)
            panels.append(panel)
            self.pageStyle.SetPanel(panel)
            self.nb.AddPage(self.pageStyle, 'Style')
            self.SetStyleValues(panel, comp.getAttribute(node, 'style'))

        if comp.exStyles or comp.genericExStyles:
            # Create extra style page
            panel = params.StylePanel(self.pageExStyle, comp.exStyles + comp.genericExStyles, 
                                      tag='exstyle')
            panels.append(panel)
            self.pageExStyle.SetPanel(panel)
            self.nb.AddPage(self.pageExStyle, 'ExStyle')
            self.SetStyleValues(panel, comp.getAttribute(node, 'exstyle'))

        # Additional panel for hidden node
        if container and container.requireImplicit(node) and container.implicitAttributes:
            panel = AttributePanel(self.pageIA, 
                                   container.implicitAttributes, 
                                   container.implicitParams,
                                   container.implicitRenameDict)
            panel.comp = container
            panels.append(panel)
            self.pageIA.SetPanel(panel)
            self.nb.AddPage(self.pageIA, container.implicitPageName)
            self.SetValues(panel, node.parentNode)

        if comp.hasCode:
            # Create code page
            panel = CodePanel(self.pageCode, comp.events)
            panel.node = node
            panels.append(panel)
            self.pageCode.SetPanel(panel)
            self.nb.AddPage(self.pageCode, 'Code')
            self.SetCodeValues(panel, comp.getAttribute(node, 'XRCED'))

        # Select old page if possible and pin is down
        if g.conf.panelPinState:
            for i in range(1, self.nb.GetPageCount()):
                if oldLabel == self.nb.GetPageText(i):
                    self.nb.SetSelection(i)
                    break

        self.Thaw()

        return panels
        
    def Clear(self):
        self.comp = None
        self.nb.SetSelection(0)
        map(self.nb.RemovePage, range(self.nb.GetPageCount()-1, 0, -1))
        self.pageA.Reset()
        self.undo = None

        self.textClass.SetValue('')
        self.labelName.Show(False)
        self.textName.Show(False)

        self.Layout()

    def GetActivePanel(self):
        if self.nb.GetSelection() >= 0:
            return self.nb.GetPage(self.nb.GetSelection()).panel
        else:
            return None

    # Set data for a panel
    def SetValues(self, panel, node):
        panel.node = node
        if isinstance(panel, AttributePanel) and panel.comp:
            comp = panel.comp
        else:
            comp = self.comp
        for a,w in panel.controls:
            value = comp.getAttribute(node, a)
            w.SetValue(value)

    # Set data for a style panel
    def SetStyleValues(self, panel, style):
        panel.style = style
        panel.node = self.node
        styles = map(string.strip, style.split('|')) # to list
        for s,w in panel.controls:
            w.SetValue(s in styles)

    # Set data for a style panel
    def SetCodeValues(self, panel, data):
        panel.SetValues([('XRCED', data)])


################################################################################

class AttributePanel(wx.Panel):
    '''Particular attribute panel, normally inside a notebook.'''
    def __init__(self, parent, attributes, param_dict={}, rename_dict={}):
        wx.Panel.__init__(self, parent, -1)
        self.controls = []
        self.comp = None                # if not None overrides default component
        sizer = wx.FlexGridSizer(len(attributes), 2, 0, 0)
        sizer.AddGrowableCol(1, 0)
        for a in attributes:
            # Find good control class
            paramClass = param_dict.get(a, params.paramDict.get(a, params.ParamText))
            sParam = rename_dict.get(a, a)
            control = paramClass(self, sParam)
            labelPanel = wx.Panel(self, -1)
            labelSizer = wx.BoxSizer()
            labelPanel.SetSizer(labelSizer)
            if control.isCheck: # checkbox-like control
                label = wx.StaticText(labelPanel, -1, control.defaultString)
                sizer.AddMany([ (control, 1, wx.EXPAND),
                                (labelPanel, 1, wx.EXPAND) ])
                labelSizer.Add(label, 0, wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 2)
            else:
                if sParam:
                    label = wx.StaticText(labelPanel, -1, sParam, size=labelSize)
                    sizer.AddMany([ (labelPanel, 1, wx.EXPAND),
                                    (control, 1, wx.EXPAND) ])
                else:           # for node-level params
                    label = wx.StaticText(labelPanel, -1, '')
                    sizer.Add(control, 1, wx.LEFT, 20)
                labelSizer.Add(label, 0, wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 20)
            self.controls.append((a, control))
        self.SetSizerAndFit(sizer)

    def GetValues(self):
        '''Generic method used for creating XML and for other operations.'''
        return [(a,c.GetValue()) for a,c in self.controls]
        
    def SetValues(self, values):
        '''Generic method used for undo.'''
        for ac,a2v in zip(self.controls, values):
            a,c = ac
            v = a2v[1]
            c.SetValue(v)

################################################################################

class CodePanel(wx.Panel):
    ID_BUTTON_DEL = wx.NewId()
    ID_COMBO_EVENT = wx.NewId()
    ART_REMOVE = 'ART_REMOVE'
    
    '''Code generation panel.'''
    def __init__(self, parent, events):
        wx.Panel.__init__(self, parent, -1)
        self.SetFont(g.smallerFont())
        self.events = events
        self.checks = []
        topSizer = wx.BoxSizer(wx.HORIZONTAL)
        # Events on the left
        leftSizer = wx.BoxSizer(wx.VERTICAL)
        sizer = wx.GridSizer(cols=1, vgap=0, hgap=5)
        label = wx.StaticText(self, label='Events')
        label.SetFont(g.labelFont())
        sizer.Add(label, 0, wx.LEFT, 20)
        for ev in events:
            check = wx.CheckBox(self, label=ev)
            sizer.Add(check)
            self.checks.append((ev, check))
        leftSizer.Add(sizer)
        # Additional comboboxes
        self.extra = []
        self.eventSizer = wx.FlexGridSizer(cols=2)
        leftSizer.Add(self.eventSizer)
        topSizer.Add(leftSizer)
        # Right sizer
        rightSizer = wx.BoxSizer(wx.VERTICAL)
        rightSizer.Add((0, 10))
        if g.Presenter.container is not component.Manager.rootComponent:
            self.checkVar = wx.CheckBox(self, label='assign variable')
            rightSizer.Add(self.checkVar, 0, wx.LEFT, 20)
        else:
            self.checkVar = None
        topSizer.Add(rightSizer)
        # Cach all checkbox events
        self.Bind(wx.EVT_CHECKBOX, self.OnCheck)
        self.SetSizerAndFit(topSizer)
        # Extra combos and buttons
        self.Bind(wx.EVT_BUTTON, self.OnButtonDel, id=self.ID_BUTTON_DEL)
        self.Bind(wx.EVT_COMBOBOX, self.OnComboEvent, id=self.ID_COMBO_EVENT)
        self.Bind(wx.EVT_TEXT, self.OnComboText, id=self.ID_COMBO_EVENT)

    def GetValues(self):
        events = []
        for s,check in self.checks:
            if check.IsChecked(): events.append(s)
        # Encode data to a dictionary and the cPicke it
        data = {}
        for btn,combo in self.extra[:-1]:
            events.append(combo.GetValue())
        if events: data['events'] = '|'.join(events)
        if self.checkVar and self.checkVar.GetValue(): data['assign_var'] = '1'
        if data:
            return [('XRCED', data)]
        else:
            return []

    def AddExtraEvent(self, event=''):
        btn = wx.BitmapButton(self, self.ID_BUTTON_DEL,
                              bitmap=wx.ArtProvider.GetBitmap(self.ART_REMOVE, wx.ART_BUTTON),
                              size=(20,20))
        if not event: btn.Disable()
        self.eventSizer.Add(btn, 0, wx.ALIGN_CENTRE_VERTICAL)
        combo = wx.ComboBox(self, self.ID_COMBO_EVENT, value=event, choices=component.Component.genericEvents)
        btn.combo = combo
        self.eventSizer.Add(combo)
        self.extra.append((btn, combo))

    def SetValues(self, values):
        data = values[0][1]
        events = data.get('events', '').split('|')
        if events == ['']: events = []
        for ev,check in self.checks:
            check.SetValue(ev in events)
        # Add comboboxes for other events
        for ev in events:
            if ev not in self.events:
                self.AddExtraEvent(ev)
        # Empty combo box for adding new events
        self.AddExtraEvent()
        self.Fit()
        self.SetMinSize(self.GetBestSize())
        if self.checkVar:
            self.checkVar.SetValue(int(data.get('assign_var', '0')))

    def OnCheck(self, evt):
        g.Presenter.setApplied(False)

    def OnButtonDel(self, evt):
        btn = evt.GetEventObject()
        self.extra.remove((btn, btn.combo))
        btn.combo.Destroy()
        btn.Destroy()
        self.eventSizer.Layout()
        self.Fit()
        self.SetMinSize(self.GetBestSize())
        g.Presenter.setApplied(False)

    def OnComboText(self, evt):
        if evt.GetEventObject() == self.extra[-1][1]:
            self.extra[-1][0].Enable()
            self.AddExtraEvent()
            self.eventSizer.Layout()
            self.Fit()
            self.SetMinSize(self.GetBestSize())
        g.Presenter.setApplied(False)

    def OnComboEvent(self, evt):
        if evt.GetEventObject() == self.extra[-1][1]:
            self.extra[-1][0].Enable()
            self.AddExtraEvent()
            self.eventSizer.Layout()
            self.Fit()
            self.SetMinSize(self.GetBestSize())
        g.Presenter.setApplied(False)
            
