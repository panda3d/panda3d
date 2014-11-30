# Name:         tools.py
# Purpose:      XRC editor, toolbar
# Author:       Roman Rolinsky <rolinsky@mema.ucl.ac.be>
# Created:      19.03.2003
# RCS-ID:       $Id: tools.py 67456 2011-04-13 18:02:41Z RD $

from globals import *
from component import Manager, DEFAULT_POS
import view
import images
import wx.lib.foldpanelbar as fpb

#if wx.Platform in ['__WXMAC__', '__WXMSW__']:
    # Mac and Win are better off with generic
import wx.lib.buttons
BitmapButton = wx.lib.buttons.GenBitmapButton
#else:
#    wx.BitmapButton.SetBezelWidth = lambda self, w: None

class ToolPanel(wx.PyPanel):
    '''Manages a Listbook with tool bitmap buttons.'''
    defaultPos = wx.GBPosition(*DEFAULT_POS)
    def __init__(self, parent):
        if wx.Platform == '__WXGTK__':
            wx.PyPanel.__init__(self, parent, -1,
                             style=wx.RAISED_BORDER|wx.WANTS_CHARS)
        else:
            wx.PyPanel.__init__(self, parent, -1, style=wx.WANTS_CHARS)
        self.bg = wx.Colour(115, 180, 215)
        # Top sizer
        sizer = wx.BoxSizer(wx.VERTICAL)
        # Use toolbook or foldpanelbar depending of preferences
        if g.conf.toolPanelType == 'TB':
            self.tp = wx.Toolbook(self, -1, style=wx.BK_TOP)
            sizer.Add(self.tp, 1, wx.EXPAND)
            # Image list
            thumbSize = g.conf.toolThumbSize
            il = wx.ImageList(thumbSize, thumbSize, True)
            # Default Id 0
            il.Add(images.ToolPanel_Default.GetImage().Scale(thumbSize, thumbSize).ConvertToBitmap())
            self.il = il
            self.tp.AssignImageList(il)
        elif g.conf.toolPanelType == 'FPB':
            self.tp = fpb.FoldPanelBar(self, -1, wx.DefaultPosition, wx.DefaultSize, 
                                       agwStyle=fpb.FPB_VERTICAL)
            sizer.Add(self.tp, 1, wx.EXPAND)
        self.panels = []
        for name in Manager.panelNames:
            panelData = Manager.getPanelData(name)
            if not panelData: continue
            try:
                im = Manager.panelImages[name]
                imageId = il.Add(im.Scale(thumbSize, thumbSize).ConvertToBitmap())
            except:
                imageId = 0
            panel = self.AddPanel(name)
            self.panels.append(panel)
            for pos,span,comp,bmp in panelData:
                self.AddButton(panel, pos, span, comp.id, bmp, comp.klass)
            panel.Fit()
            if g.conf.toolPanelType == 'TB':
                self.tp.AddPage(panel, '', imageId=imageId)
            else:
                p = self.tp.AddFoldPanel(name, collapsed=False)
                p.SetBackgroundColour(self.bg)
                panel.Reparent(p)
                p.AddWindow(panel, fpb.FPB_ALIGN_WIDTH)
        self.tp.Fit()
        
        self.SetSizer(sizer)
        # Allow to be resized in horizontal direction only
        # Events
#        wx.EVT_KEY_DOWN(self, self.OnKeyDown)
#        wx.EVT_KEY_UP(self, self.OnKeyUp)
        self.drag = None

    def DoGetBestSize(self):
        # Do our own DoGetBestSize because the FoldPanelBar doesn't
        h = w = 0
        for p in self.panels:
            ems = p.GetEffectiveMinSize()
            w = max(w, ems.width)
            h = max(h, ems.height)
        h += 64
        return wx.Size(w,h)

    def AddButton(self, panel, pos, span, id, bmp, text):
        button = BitmapButton(panel, id, bmp, 
                              style=wx.NO_BORDER)# | wx.WANTS_CHARS)
        button.SetBezelWidth(0)
#        wx.EVT_KEY_DOWN(button, self.OnKeyDown)
#        wx.EVT_KEY_UP(button, self.OnKeyUp)
        button.Bind(wx.EVT_LEFT_DOWN, self.OnLeftDownOnButton)
        button.Bind(wx.EVT_MOTION, self.OnMotionOnButton)
        button.Bind(wx.EVT_BUTTON, self.OnButton)
        button.SetToolTipString(text)
        # Look for an available place if not specified
        r0,c0 = 0,0
        if pos != self.defaultPos:
            if panel.sizer.CheckForIntersectionPos(pos, span):
                r0,c0 = pos     # start from pos
                pos = self.defaultPos   # reset to default
        if pos == self.defaultPos:
            # Find the smallest position we can insert into
            try:
                for r in range(r0, panel.size.rowspan):
                    for c in range(c0, panel.size.colspan):
                        if not panel.sizer.CheckForIntersectionPos((r,c), span):
                            pos = (r,c)
                            raise StopIteration
            except StopIteration:
                pass
            if pos == self.defaultPos:  # insert new col/row
                if panel.size.colspan + span[0] <= panel.size.rowspan + span[1]:
                    pos = (0,panel.size.colspan)
                else:
                    pos = (panel.size.rowspan,0)
        assert pos[0] >= 0 and pos[1] >= 0, 'invalid position'
        panel.sizer.Add(button, pos, span, wx.ALIGN_CENTRE)
        panel.controls[id] = button
        panel.size.rowspan = max(panel.size.rowspan, pos[0]+span[0])
        panel.size.colspan = max(panel.size.colspan, pos[1]+span[1])

    def AddPanel(self, name):
        # Each group is inside box
        panel = wx.Panel(self.tp)
        panel.SetBackgroundColour(self.bg)
        panel.name = name
        panel.controls = {}
        panel.size = wx.GBSpan(1, 1) # current size
        topSizer = wx.BoxSizer()
        panel.sizer = wx.GridBagSizer(0, 0)
        panel.sizer.SetEmptyCellSize((24, 24))
        topSizer.Add(panel.sizer, 1, wx.EXPAND | wx.ALL, 5)
        panel.SetSizer(topSizer)
        return panel

    # Mouse events for DnD and append/insert mode
    def OnLeftDownOnButton(self, evt):
        self.posDown = evt.GetPosition()
        self.btnDown = evt.GetEventObject()
        forceSibling = evt.ControlDown()
        forceInsert = evt.ShiftDown()
        g.Presenter.updateCreateState(forceSibling, forceInsert)
        evt.Skip()

    def OnButton(self, evt):
        if not self.drag: evt.Skip()
        self.drag = False
        if view.frame.miniFrame:
            if not g.useAUI and not g.conf.embedPanel:
                view.frame.miniFrame.Raise()
        else:
            view.frame.Raise()

    def OnMotionOnButton(self, evt):
        # Detect dragging
        if evt.Dragging() and evt.LeftIsDown():
            d = evt.GetPosition() - self.posDown
            if max(abs(d[0]), abs(d[1])) >= 5:
                if self.btnDown.HasCapture(): 
                    # Generate up event to release mouse
                    evt = wx.MouseEvent(wx.EVT_LEFT_UP.typeId)
                    #evt.SetId(self.btnDown.GetId())
                    # Set flag to prevent normal button operation
                    self.drag = True
                    self.btnDown.ProcessEvent(evt)
                self.StartDrag()
        evt.Skip()

    def StartDrag(self):
        bm = self.btnDown.GetBitmapLabel()
        # wxGTK requires wxIcon cursor, wxWIN and wxMAC require wxCursor
        if wx.Platform == '__WXGTK__':
            icon = wx.EmptyIcon()
            icon.CopyFromBitmap(bm)
            dragSource = wx.DropSource(self, icon)
        else:
            curs = wx.CursorFromImage(wx.ImageFromBitmap(bm))
            dragSource = wx.DropSource(self, curs)
        do = MyDataObject(str(self.btnDown.GetId()))
        dragSource.SetData(do)
        view.frame.SetStatusText('Release the mouse button over the test window')
        dragSource.DoDragDrop()
        view.testWin.RemoveHighlightDT()
        view.testWin.EmptyTrash()

    # Process key events
    def OnKeyDown(self, evt):
        print evt.GetEventObject(), evt.GetKeyCode()
        evt.Skip()
        return
        if evt.GetKeyCode() == wx.WXK_CONTROL:
            g.tree.ctrl = True
        elif evt.GetKeyCode() == wx.WXK_SHIFT:
            g.tree.shift = True
        self.UpdateIfNeeded()
        evt.Skip()

    def OnKeyUp(self, evt):
        if evt.GetKeyCode() == wx.WXK_CONTROL:
            g.tree.ctrl = False
        elif evt.GetKeyCode() == wx.WXK_SHIFT:
            g.tree.shift = False
        self.UpdateIfNeeded()
        evt.Skip()

    def OnMouse(self, evt):
        # Update control and shift states
        g.tree.ctrl = evt.ControlDown()
        g.tree.shift = evt.ShiftDown()
        self.UpdateIfNeeded()
        evt.Skip()

    # Update UI after key presses, if necessary
    def UpdateIfNeeded(self):
        tree = g.tree
        if self.ctrl != tree.ctrl or self.shift != tree.shift:
            # Enabling is needed only for ctrl
            if self.ctrl != tree.ctrl: self.UpdateUI()
            self.ctrl = tree.ctrl
            self.shift = tree.shift
            if tree.ctrl:
                status = 'SBL'
            elif tree.shift:
                status = 'INS'
            else:
                status = ''
            g.frame.SetStatusText(status, 1)

