#----------------------------------------------------------------------------
# Name:        wx.lib.inspection
# Purpose:     A widget inspection tool that allows easy introspection of
#              all the live widgets and sizers in an application.
#
# Author:      Robin Dunn
#
# Created:     26-Jan-2007
# RCS-ID:      $Id: inspection.py 63676 2010-03-12 23:53:48Z RD $
# Copyright:   (c) 2007 by Total Control Software
# Licence:     wxWindows license
#----------------------------------------------------------------------------

# NOTE: This class was originally based on ideas sent to the
# wxPython-users mail list by Dan Eloff.  See also
# wx.lib.mixins.inspect for a class that can be mixed-in with wx.App
# to provide Hot-Key access to the inspection tool.

import wx
import wx.py
import wx.stc
import wx.aui
import wx.lib.utils as utils
import sys
import inspect

#----------------------------------------------------------------------------

class InspectionTool:
    """
    The InspectionTool is a singleton that manages creating and
    showing an InspectionFrame.
    """

    # Note: This is the Borg design pattern which ensures that all
    # instances of this class are actually using the same set of
    # instance data.  See
    # http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/66531
    __shared_state = {}
    def __init__(self):
        self.__dict__ = self.__shared_state
        if not hasattr(self, 'initialized'):
            self.initialized = False

    def Init(self, pos=wx.DefaultPosition, size=wx.Size(850,700),
             config=None, locals=None, app=None):
        """
        Init is used to set some parameters that will be used later
        when the inspection tool is shown.  Suitable defaults will be
        used for all of these parameters if they are not provided.

        :param pos:   The default position to show the frame at
        :param size:  The default size of the frame
        :param config: A wx.Config object to be used to store layout
            and other info to when the inspection frame is closed.
            This info will be restored the next time the inspection
            frame is used.
        :param locals: A dictionary of names to be added to the PyCrust
            namespace.
        :param app:  A reference to the wx.App object.
        """
        self._frame = None
        self._pos = pos
        self._size = size
        self._config = config
        self._locals = locals
        self._app = app
        if not self._app:
            self._app = wx.GetApp()
        self.initialized = True


    def Show(self, selectObj=None, refreshTree=False):
        """
        Creates the inspection frame if it hasn't been already, and
        raises it if neccessary.  Pass a widget or sizer in selectObj
        to have that object be preselected in widget tree.  If
        refreshTree is True then the widget tree will be rebuilt,
        otherwise if the tree has already been built it will be left
        alone.
        """
        if not self.initialized:
            self.Init()

        parent = self._app.GetTopWindow()
        if not selectObj:
            selectObj = parent
        if not self._frame:
            self._frame = InspectionFrame( parent=parent,
                                           pos=self._pos,
                                           size=self._size,
                                           config=self._config,
                                           locals=self._locals,
                                           app=self._app)
        if selectObj:
            self._frame.SetObj(selectObj)
        if refreshTree:
            self._frame.RefreshTree()
        self._frame.Show()
        if self._frame.IsIconized():
            self._frame.Iconize(False)
        self._frame.Raise()


#----------------------------------------------------------------------------


class InspectionFrame(wx.Frame):
    """
    This class is the frame that holds the wxPython inspection tools.
    The toolbar and AUI splitters/floating panes are also managed
    here.  The contents of the tool windows are handled by other
    classes.
    """
    def __init__(self, wnd=None, locals=None, config=None,
                 app=None, title="wxPython Widget Inspection Tool",
                 *args, **kw):
        kw['title'] = title
        wx.Frame.__init__(self, *args, **kw)

        self.SetExtraStyle(wx.WS_EX_BLOCK_EVENTS)
        self.includeSizers = False
        self.started = False

        self.SetIcon(Icon.GetIcon())
        self.MakeToolBar()
        panel = wx.Panel(self, size=self.GetClientSize())

        # tell FrameManager to manage this frame
        self.mgr = wx.aui.AuiManager(panel,
                                     wx.aui.AUI_MGR_DEFAULT
                                     | wx.aui.AUI_MGR_TRANSPARENT_DRAG
                                     | wx.aui.AUI_MGR_ALLOW_ACTIVE_PANE)

        # make the child tools
        self.tree = InspectionTree(panel, size=(100,300))
        self.info = InspectionInfoPanel(panel,
                                        style=wx.NO_BORDER,
                                        )

        if not locals:
            locals = {}
        myIntroText = (
            "Python %s on %s, wxPython %s\n"
            "NOTE: The 'obj' variable refers to the object selected in the tree."
            % (sys.version.split()[0], sys.platform, wx.version()))
        self.crust = wx.py.crust.Crust(panel, locals=locals,
                                       intro=myIntroText,
                                       showInterpIntro=False,
                                       style=wx.NO_BORDER,
                                       )
        self.locals = self.crust.shell.interp.locals
        self.crust.shell.interp.introText = ''
        self.locals['obj'] = self.obj = wnd
        self.locals['app'] = app
        self.locals['wx'] = wx
        wx.CallAfter(self._postStartup)

        # put the chlid tools in AUI panes
        self.mgr.AddPane(self.info,
                         wx.aui.AuiPaneInfo().Name("info").Caption("Object Info").
                         CenterPane().CaptionVisible(True).
                         CloseButton(False).MaximizeButton(True)
                         )
        self.mgr.AddPane(self.tree,
                         wx.aui.AuiPaneInfo().Name("tree").Caption("Widget Tree").
                         CaptionVisible(True).Left().Dockable(True).Floatable(True).
                         BestSize((280,200)).CloseButton(False).MaximizeButton(True)
                         )
        self.mgr.AddPane(self.crust,
                         wx.aui.AuiPaneInfo().Name("crust").Caption("PyCrust").
                         CaptionVisible(True).Bottom().Dockable(True).Floatable(True).
                         BestSize((400,200)).CloseButton(False).MaximizeButton(True)
                         )

        self.mgr.Update()

        if config is None:
            config = wx.Config('wxpyinspector')
        self.config = config
        self.Bind(wx.EVT_CLOSE, self.OnClose)
        self.LoadSettings(self.config)
        self.crust.shell.lineNumbers = False
        self.crust.shell.setDisplayLineNumbers(False)
        self.crust.shell.SetMarginWidth(1, 0)


    def MakeToolBar(self):
        tbar = self.CreateToolBar(wx.TB_HORIZONTAL | wx.TB_FLAT | wx.TB_TEXT | wx.NO_BORDER )
        tbar.SetToolBitmapSize((24,24))

        refreshBmp = Refresh.GetBitmap()
        findWidgetBmp = Find.GetBitmap()
        showSizersBmp = ShowSizers.GetBitmap()
        expandTreeBmp = ExpandTree.GetBitmap()
        collapseTreeBmp = CollapseTree.GetBitmap()
        highlightItemBmp = HighlightItem.GetBitmap()
        evtWatcherBmp = EvtWatcher.GetBitmap()
        
        toggleFillingBmp = ShowFilling.GetBitmap()

        refreshTool = tbar.AddLabelTool(-1, 'Refresh', refreshBmp,
                                        shortHelp = 'Refresh widget tree (F1)')
        findWidgetTool = tbar.AddLabelTool(-1, 'Find', findWidgetBmp,
                                           shortHelp='Find new target widget. (F2)  Click here and\nthen on another widget in the app.')
        showSizersTool = tbar.AddLabelTool(-1, 'Sizers', showSizersBmp,
                                           shortHelp='Include sizers in widget tree (F3)',
                                           kind=wx.ITEM_CHECK)
        expandTreeTool = tbar.AddLabelTool(-1, 'Expand', expandTreeBmp,
                                           shortHelp='Expand all tree items (F4)')
        collapseTreeTool = tbar.AddLabelTool(-1, 'Collapse', collapseTreeBmp,
                                            shortHelp='Collapse all tree items (F5)')
        highlightItemTool = tbar.AddLabelTool(-1, 'Highlight', highlightItemBmp,
                                           shortHelp='Attempt to highlight live item (F6)')
        evtWatcherTool = tbar.AddLabelTool(-1, 'Events', evtWatcherBmp,
                                           shortHelp='Watch the events of the selected item (F7)')
        
        toggleFillingTool = tbar.AddLabelTool(-1, 'Filling', toggleFillingBmp,
                                              shortHelp='Show PyCrust \'filling\' (F8)',
                                              kind=wx.ITEM_CHECK)
        tbar.Realize()

        self.Bind(wx.EVT_TOOL,      self.OnRefreshTree,     refreshTool)
        self.Bind(wx.EVT_TOOL,      self.OnFindWidget,      findWidgetTool)
        self.Bind(wx.EVT_TOOL,      self.OnShowSizers,      showSizersTool)
        self.Bind(wx.EVT_TOOL,      self.OnExpandTree,      expandTreeTool)
        self.Bind(wx.EVT_TOOL,      self.OnCollapseTree,    collapseTreeTool)
        self.Bind(wx.EVT_TOOL,      self.OnHighlightItem,   highlightItemTool)
        self.Bind(wx.EVT_TOOL,      self.OnWatchEvents,     evtWatcherTool)
        self.Bind(wx.EVT_TOOL,      self.OnToggleFilling,   toggleFillingTool)
        self.Bind(wx.EVT_UPDATE_UI, self.OnShowSizersUI,    showSizersTool)
        self.Bind(wx.EVT_UPDATE_UI, self.OnWatchEventsUI,   evtWatcherTool)
        self.Bind(wx.EVT_UPDATE_UI, self.OnToggleFillingUI, toggleFillingTool)

        tbl = wx.AcceleratorTable(
            [(wx.ACCEL_NORMAL, wx.WXK_F1, refreshTool.GetId()),
             (wx.ACCEL_NORMAL, wx.WXK_F2, findWidgetTool.GetId()),
             (wx.ACCEL_NORMAL, wx.WXK_F3, showSizersTool.GetId()),
             (wx.ACCEL_NORMAL, wx.WXK_F4, expandTreeTool.GetId()),
             (wx.ACCEL_NORMAL, wx.WXK_F5, collapseTreeTool.GetId()),
             (wx.ACCEL_NORMAL, wx.WXK_F6, highlightItemTool.GetId()),
             (wx.ACCEL_NORMAL, wx.WXK_F7, evtWatcherTool.GetId()),
             (wx.ACCEL_NORMAL, wx.WXK_F8, toggleFillingTool.GetId()),
             ])
        self.SetAcceleratorTable(tbl)


    def _postStartup(self):
        if self.crust.ToolsShown():
            self.crust.ToggleTools()
        self.UpdateInfo()
        self.started = True


    def OnClose(self, evt):
        self.SaveSettings(self.config)
        self.mgr.UnInit()
        del self.mgr
        evt.Skip()


    def UpdateInfo(self):
        self.info.Update(self.obj)


    def SetObj(self, obj):
        if self.obj is obj:
            return
        self.locals['obj'] = self.obj = obj
        self.UpdateInfo()
        if not self.tree.built:
            self.tree.BuildTree(obj, includeSizers=self.includeSizers)
        else:
            self.tree.SelectObj(obj)


    def HighlightCurrentItem(self):
        """
        Draw a highlight rectangle around the item represented by the
        current tree selection.
        """
        if not hasattr(self, 'highlighter'):
            self.highlighter = _InspectionHighlighter()
        self.highlighter.HighlightCurrentItem(self.tree)


    def RefreshTree(self):
        self.tree.BuildTree(self.obj, includeSizers=self.includeSizers)


    def OnRefreshTree(self, evt):
        self.RefreshTree()
        self.UpdateInfo()


    def OnFindWidget(self, evt):
        self.Bind(wx.EVT_LEFT_DOWN, self.OnLeftDown)
        self.Bind(wx.EVT_MOUSE_CAPTURE_LOST, self.OnCaptureLost)
        self.CaptureMouse()
        self.finding = wx.BusyInfo("Click on any widget in the app...")


    def OnCaptureLost(self, evt):
        self.Unbind(wx.EVT_LEFT_DOWN)
        self.Unbind(wx.EVT_MOUSE_CAPTURE_LOST)
        del self.finding

    def OnLeftDown(self, evt):
        self.ReleaseMouse()
        wnd = wx.FindWindowAtPointer()
        if wnd is not None:
            self.SetObj(wnd)
        else:
            wx.Bell()
        self.OnCaptureLost(evt)


    def OnShowSizers(self, evt):
        self.includeSizers = not self.includeSizers
        self.RefreshTree()


    def OnExpandTree(self, evt):
        current = self.tree.GetSelection()
        self.tree.ExpandAll()
        self.tree.EnsureVisible(current)


    def OnCollapseTree(self, evt):
        current = self.tree.GetSelection()
        self.tree.CollapseAll()
        self.tree.EnsureVisible(current)
        self.tree.SelectItem(current)


    def OnHighlightItem(self, evt):
        self.HighlightCurrentItem()

        
    def OnWatchEvents(self, evt):
        item = self.tree.GetSelection()
        obj = self.tree.GetItemPyData(item)
        if isinstance(obj, wx.Window):
            import wx.lib.eventwatcher as ew
            watcher = ew.EventWatcher(self)
            watcher.watch(obj)
            watcher.Show()
            
    def OnWatchEventsUI(self, evt):
        item = self.tree.GetSelection()
        if item:
            obj = self.tree.GetItemPyData(item)
            evt.Enable(isinstance(obj, wx.Window))


    def OnToggleFilling(self, evt):
        self.crust.ToggleTools()


    def OnShowSizersUI(self, evt):
        evt.Check(self.includeSizers)


    def OnToggleFillingUI(self, evt):
        if self.started:
            evt.Check(self.crust.ToolsShown())


    def LoadSettings(self, config):
        self.crust.LoadSettings(config)
        self.info.LoadSettings(config)

        pos  = wx.Point(config.ReadInt('Window/PosX', -1),
                        config.ReadInt('Window/PosY', -1))

        size = wx.Size(config.ReadInt('Window/Width', -1),
                       config.ReadInt('Window/Height', -1))
        self.SetSize(size)
        self.Move(pos)
        rect = utils.AdjustRectToScreen(self.GetRect())
        self.SetRect(rect)
        
        perspective = config.Read('perspective', '')
        if perspective:
            self.mgr.LoadPerspective(perspective)
        self.includeSizers = config.ReadBool('includeSizers', False)


    def SaveSettings(self, config):
        self.crust.SaveSettings(config)
        self.info.SaveSettings(config)

        if not self.IsIconized() and not self.IsMaximized():
            w, h = self.GetSize()
            config.WriteInt('Window/Width', w)
            config.WriteInt('Window/Height', h)

            px, py = self.GetPosition()
            config.WriteInt('Window/PosX', px)
            config.WriteInt('Window/PosY', py)

        perspective = self.mgr.SavePerspective()
        config.Write('perspective', perspective)
        config.WriteBool('includeSizers', self.includeSizers)

#---------------------------------------------------------------------------

# should inspection frame (and children) be includeed in the tree?
INCLUDE_INSPECTOR = True

USE_CUSTOMTREECTRL = False
if USE_CUSTOMTREECTRL:
    import wx.lib.customtreectrl as CT
    TreeBaseClass = CT.CustomTreeCtrl
else:
    TreeBaseClass = wx.TreeCtrl

class InspectionTree(TreeBaseClass):
    """
    All of the widgets in the app, and optionally their sizers, are
    loaded into this tree.
    """
    def __init__(self, *args, **kw):
        #s = kw.get('style', 0)
        #kw['style'] = s | wx.TR_DEFAULT_STYLE | wx.TR_HIDE_ROOT
        TreeBaseClass.__init__(self, *args, **kw)
        self.roots = []
        self.built = False
        self.Bind(wx.EVT_TREE_SEL_CHANGED, self.OnSelectionChanged)
        self.toolFrame = wx.GetTopLevelParent(self)
        if 'wxMac' in wx.PlatformInfo:
            self.SetWindowVariant(wx.WINDOW_VARIANT_SMALL)


    def BuildTree(self, startWidget, includeSizers=False, expandFrame=False):
        if self.GetCount():
            self.DeleteAllItems()
            self.roots = []
            self.built = False

        realRoot = self.AddRoot('Top-level Windows')

        for w in wx.GetTopLevelWindows():
            if w is wx.GetTopLevelParent(self) and not INCLUDE_INSPECTOR:
                continue
            root  = self._AddWidget(realRoot, w, includeSizers)
            self.roots.append(root)

        # Expand the subtree containing the startWidget, and select it.
        if not startWidget or not isinstance(startWidget, wx.Window):
            startWidget = wx.GetApp().GetTopWindow()
        if expandFrame:
            top = wx.GetTopLevelParent(startWidget)
            topItem = self.FindWidgetItem(top)
            if topItem:
                self.ExpandAllChildren(topItem)
        self.SelectObj(startWidget)
        self.built = True


    def _AddWidget(self, parentItem, widget, includeSizers):
        text = self.GetTextForWidget(widget)
        item = self.AppendItem(parentItem, text)
        self.SetItemPyData(item, widget)

        # Add the sizer and widgets in the sizer, if we're showing them
        widgetsInSizer = []
        if includeSizers and widget.GetSizer() is not None:
            widgetsInSizer = self._AddSizer(item, widget.GetSizer())

        # Add any children not in the sizer, or all children if we're
        # not showing the sizers
        for child in widget.GetChildren():
            if (not child in widgetsInSizer and
                (not child.IsTopLevel() or
                 isinstance(child, wx.PopupWindow))):
                self._AddWidget(item, child, includeSizers)

        return item


    def _AddSizer(self, parentItem, sizer):
        widgets = []
        text = self.GetTextForSizer(sizer)
        item = self.AppendItem(parentItem, text)
        self.SetItemPyData(item, sizer)
        self.SetItemTextColour(item, "blue")

        for si in sizer.GetChildren():
            if si.IsWindow():
                w = si.GetWindow()
                self._AddWidget(item, w, True)
                widgets.append(w)
            elif si.IsSizer():
                ss = si.GetSizer()
                widgets += self._AddSizer(item, ss)
                ss._parentSizer = sizer
            else:
                i = self.AppendItem(item, "Spacer")
                self.SetItemPyData(i, si)
                self.SetItemTextColour(i, "blue")
        return widgets


    def FindWidgetItem(self, widget):
        """
        Find the tree item for a widget.
        """
        for item in self.roots:
            found = self._FindWidgetItem(widget, item)
            if found:
                return found
        return None

    def _FindWidgetItem(self, widget, item):
        if self.GetItemPyData(item) is widget:
            return item
        child, cookie = self.GetFirstChild(item)
        while child:
            found = self._FindWidgetItem(widget, child)
            if found:
                return found
            child, cookie = self.GetNextChild(item, cookie)
        return None


    def GetTextForWidget(self, widget):
        """
        Returns the string to be used in the tree for a widget
        """
        return "%s (\"%s\")" % (widget.__class__.__name__, widget.GetName())

    
    def GetTextForSizer(self, sizer):
        """
        Returns the string to be used in the tree for a sizer
        """
        return "%s" % sizer.__class__.__name__


    def SelectObj(self, obj):
        item = self.FindWidgetItem(obj)
        if item:
            self.EnsureVisible(item)
            self.SelectItem(item)


    def OnSelectionChanged(self, evt):
        obj = self.GetItemPyData(evt.GetItem())
        self.toolFrame.SetObj(obj)


#---------------------------------------------------------------------------

class InspectionInfoPanel(wx.stc.StyledTextCtrl):
    """
    Used to display information about the currently selected items.
    Currently just a read-only wx.stc.StyledTextCtrl with some plain
    text.  Should probably add some styles to make things easier to
    read.
    """
    def __init__(self, *args, **kw):
        wx.stc.StyledTextCtrl.__init__(self, *args, **kw)

        from wx.py.editwindow import FACES
        self.StyleSetSpec(wx.stc.STC_STYLE_DEFAULT,
                          "face:%(mono)s,size:%(size)d,back:%(backcol)s" % FACES)
        self.StyleClearAll()
        self.SetReadOnly(True)
        self.SetMarginType(1, 0)
        self.SetMarginWidth(1, 0)
        self.SetSelForeground(True, wx.SystemSettings.GetColour(wx.SYS_COLOUR_HIGHLIGHTTEXT))
        self.SetSelBackground(True, wx.SystemSettings.GetColour(wx.SYS_COLOUR_HIGHLIGHT))


    def LoadSettings(self, config):
        zoom = config.ReadInt('View/Zoom/Info', 0)
        self.SetZoom(zoom)

    def SaveSettings(self, config):
        config.WriteInt('View/Zoom/Info', self.GetZoom())


    def Update(self, obj):
        st = []
        if not obj:
            st.append("Item is None or has been destroyed.")

        elif isinstance(obj, wx.Window):
            st += self.FmtWidget(obj)

        elif isinstance(obj, wx.Sizer):
            st += self.FmtSizer(obj)

        elif isinstance(obj, wx.SizerItem):
            st += self.FmtSizerItem(obj)

        self.SetReadOnly(False)
        self.SetText('\n'.join(st))
        self.SetReadOnly(True)


    def Fmt(self, name, value):
        if isinstance(value, (str, unicode)):
            return "    %s = '%s'" % (name, value)
        else:
            return "    %s = %s" % (name, value)


    def FmtWidget(self, obj):
        def _countChildren(children):
            count = 0
            for child in children:
                if not child.IsTopLevel():
                    count += 1
                    count += _countChildren(child.GetChildren())
            return count
        def _countAllChildren(children):
            count = 0
            for child in children:
                count += 1
                count += _countAllChildren(child.GetChildren())
            return count

        count = len([c for c in obj.GetChildren() if not c.IsTopLevel()])
        rcount = _countChildren(obj.GetChildren())
        tlwcount = _countAllChildren(obj.GetChildren())

        st = ["Widget:"]
        st.append(self.Fmt('name',        obj.GetName()))
        st.append(self.Fmt('class',       obj.__class__))
        st.append(self.Fmt('bases',       obj.__class__.__bases__))
        st.append(self.Fmt('module',      inspect.getmodule(obj)))
        if hasattr(obj, 'this'):
            st.append(self.Fmt('this',    repr(obj.this)))
        st.append(self.Fmt('id',          obj.GetId()))
        st.append(self.Fmt('style',       obj.GetWindowStyle()))
        st.append(self.Fmt('pos',         obj.GetPosition()))
        st.append(self.Fmt('size',        obj.GetSize()))
        st.append(self.Fmt('minsize',     obj.GetMinSize()))
        st.append(self.Fmt('bestsize',    obj.GetBestSize()))
        st.append(self.Fmt('client size', obj.GetClientSize()))
        st.append(self.Fmt('virtual size',obj.GetVirtualSize()))
        st.append(self.Fmt('IsEnabled',   obj.IsEnabled()))
        st.append(self.Fmt('IsShown',     obj.IsShown()))
        st.append(self.Fmt('fg color',    obj.GetForegroundColour()))
        st.append(self.Fmt('bg color',    obj.GetBackgroundColour()))
        st.append(self.Fmt('label',       obj.GetLabel()))
        if hasattr(obj, 'GetTitle'):
            st.append(self.Fmt('title',   obj.GetTitle()))
        if hasattr(obj, 'GetValue'):
            try:
                st.append(self.Fmt('value',   obj.GetValue()))
            except:
                pass
        st.append('    child count = %d (direct)  %d (recursive)  %d (include TLWs)' %
                  (count, rcount, tlwcount))
        if obj.GetContainingSizer() is not None:
            st.append('')
            sizer = obj.GetContainingSizer()
            st += self.FmtSizerItem(sizer.GetItem(obj))
        return st


    def FmtSizerItem(self, obj):
        if obj is None:
            return ['SizerItem: None']

        st = ['SizerItem:']
        st.append(self.Fmt('proportion', obj.GetProportion()))
        st.append(self.Fmt('flag',
                           FlagsFormatter(itemFlags, obj.GetFlag())))
        st.append(self.Fmt('border',     obj.GetBorder()))
        st.append(self.Fmt('pos',        obj.GetPosition()))
        st.append(self.Fmt('size',       obj.GetSize()))
        st.append(self.Fmt('minsize',    obj.GetMinSize()))
        st.append(self.Fmt('ratio',      obj.GetRatio()))
        st.append(self.Fmt('IsWindow',   obj.IsWindow()))
        st.append(self.Fmt('IsSizer',    obj.IsSizer()))
        st.append(self.Fmt('IsSpacer',   obj.IsSpacer()))
        st.append(self.Fmt('IsShown',    obj.IsShown()))
        if isinstance(obj, wx.GBSizerItem):
            st.append(self.Fmt('cellpos',    obj.GetPos()))
            st.append(self.Fmt('cellspan',   obj.GetSpan()))
            st.append(self.Fmt('endpos',     obj.GetEndPos()))
        return st


    def FmtSizer(self, obj):
        st = ['Sizer:']
        st.append(self.Fmt('class',      obj.__class__))
        if hasattr(obj, 'this'):
            st.append(self.Fmt('this',      repr(obj.this)))
        st.append(self.Fmt('pos',        obj.GetPosition()))
        st.append(self.Fmt('size',       obj.GetSize()))
        st.append(self.Fmt('minsize',    obj.GetMinSize()))
        if isinstance(obj, wx.BoxSizer):
            st.append(self.Fmt('orientation',
                               FlagsFormatter(orientFlags, obj.GetOrientation())))
        if isinstance(obj, wx.GridSizer):
            st.append(self.Fmt('cols', obj.GetCols()))
            st.append(self.Fmt('rows', obj.GetRows()))
            st.append(self.Fmt('vgap', obj.GetVGap()))
            st.append(self.Fmt('hgap', obj.GetHGap()))
        if isinstance(obj, wx.FlexGridSizer):
            st.append(self.Fmt('rowheights', obj.GetRowHeights()))
            st.append(self.Fmt('colwidths', obj.GetColWidths()))
            st.append(self.Fmt('flexdir',
                               FlagsFormatter(orientFlags, obj.GetFlexibleDirection())))
            st.append(self.Fmt('nonflexmode',
                               FlagsFormatter(flexmodeFlags, obj.GetNonFlexibleGrowMode())))
        if isinstance(obj, wx.GridBagSizer):
            st.append(self.Fmt('emptycell', obj.GetEmptyCellSize()))

        if hasattr(obj, '_parentSizer'):
            st.append('')
            st += self.FmtSizerItem(obj._parentSizer.GetItem(obj))
            
        return st


class FlagsFormatter(object):
    def __init__(self, d, val):
        self.d = d
        self.val = val

    def __str__(self):
        st = []
        for k in self.d.keys():
            if self.val & k:
                st.append(self.d[k])
        if st:
            return '|'.join(st)
        else:
            return '0'

orientFlags = {
    wx.HORIZONTAL : 'wx.HORIZONTAL',
    wx.VERTICAL : 'wx.VERTICAL',
    }

itemFlags = {
    wx.TOP : 'wx.TOP',
    wx.BOTTOM : 'wx.BOTTOM',
    wx.LEFT : 'wx.LEFT',
    wx.RIGHT : 'wx.RIGHT',
#    wx.ALL : 'wx.ALL',
    wx.EXPAND : 'wx.EXPAND',
#    wx.GROW : 'wx.GROW',
    wx.SHAPED : 'wx.SHAPED',
    wx.STRETCH_NOT : 'wx.STRETCH_NOT',
#    wx.ALIGN_CENTER : 'wx.ALIGN_CENTER',
    wx.ALIGN_LEFT : 'wx.ALIGN_LEFT',
    wx.ALIGN_RIGHT : 'wx.ALIGN_RIGHT',
    wx.ALIGN_TOP : 'wx.ALIGN_TOP',
    wx.ALIGN_BOTTOM : 'wx.ALIGN_BOTTOM',
    wx.ALIGN_CENTER_VERTICAL : 'wx.ALIGN_CENTER_VERTICAL',
    wx.ALIGN_CENTER_HORIZONTAL : 'wx.ALIGN_CENTER_HORIZONTAL',
    wx.ADJUST_MINSIZE : 'wx.ADJUST_MINSIZE',
    wx.FIXED_MINSIZE : 'wx.FIXED_MINSIZE',
    }

flexmodeFlags = {
    wx.FLEX_GROWMODE_NONE : 'wx.FLEX_GROWMODE_NONE',
    wx.FLEX_GROWMODE_SPECIFIED : 'wx.FLEX_GROWMODE_SPECIFIED',
    wx.FLEX_GROWMODE_ALL : 'wx.FLEX_GROWMODE_ALL',
    }




#---------------------------------------------------------------------------

class _InspectionHighlighter(object):
    """
    All the highlighting code.  A separate class to help reduce the
    clutter in InspectionFrame.
    """

    # should non TLWs be flashed too?  Otherwise use a highlight rectangle
    flashAll = False

    color1 = 'red'         # for widgets and sizers
    color2 = 'red'         # for item boundaries in sizers
    color3 = '#00008B'     # for items in sizers

    highlightTime = 3000   # how long to display the highlights

    def HighlightCurrentItem(self, tree):
        """
        Draw a highlight rectangle around the item represented by the
        current tree selection.
        """
        item = tree.GetSelection()
        obj = tree.GetItemPyData(item)

        if isinstance(obj, wx.Window):
            self.HighlightWindow(obj)

        elif isinstance(obj, wx.Sizer):
            self.HighlightSizer(obj)

        elif isinstance(obj, wx.SizerItem):   # Spacer
            pItem = tree.GetItemParent(item)
            sizer = tree.GetItemPyData(pItem)
            self.HighlightSizerItem(obj, sizer)

        else:
            raise RuntimeError("unknown object type: %s" % obj.__class__.__name__)


    def HighlightWindow(self, win):
        rect = win.GetRect()
        tlw = win.GetTopLevelParent()
        if self.flashAll or tlw is win:
            self.FlickerTLW(win)
            return
        else:
            pos, useWinDC = self.FindHighlightPos(tlw, win.ClientToScreen((0,0)))
            rect.SetPosition(pos)
            self.DoHighlight(tlw, rect, self.color1, useWinDC)


    def HighlightSizerItem(self, item, sizer, penWidth=2):
        win = sizer.GetContainingWindow()
        tlw = win.GetTopLevelParent()
        rect = item.GetRect()
        pos = rect.GetPosition()
        pos, useWinDC = self.FindHighlightPos(tlw, win.ClientToScreen(pos))
        rect.SetPosition(pos)
        if rect.width < 1: rect.width = 1
        if rect.width < 1: rect.width = 1
        self.DoHighlight(tlw, rect, self.color1, useWinDC, penWidth)


    def HighlightSizer(self, sizer):
        # first do the outline of the whole sizer like normal
        win = sizer.GetContainingWindow()
        tlw = win.GetTopLevelParent()
        pos = sizer.GetPosition()
        pos, useWinDC = self.FindHighlightPos(tlw, win.ClientToScreen(pos))
        rect = wx.RectPS(pos, sizer.GetSize())
        dc = self.DoHighlight(tlw, rect, self.color1, useWinDC)

        # Now highlight the actual items within the sizer.  This may
        # get overdrawn by the code below for item boundaries, but if
        # there is border padding then this will help make it more
        # obvious.
        dc.SetPen(wx.Pen(self.color3, 1))
        for item in sizer.GetChildren():
            if item.IsShown():
                if item.IsWindow():
                    r = item.GetWindow().GetRect()
                elif item.IsSizer():
                    p = item.GetSizer().GetPosition()
                    s = item.GetSizer().GetSize()
                    r = wx.RectPS(p,s)
                else:
                    continue
                r = self.AdjustRect(tlw, win, r)
                dc.DrawRectangleRect(r)
                    
        # Next highlight the area allocated to each item in the sizer.
        # Each kind of sizer will need to be done a little
        # differently.
        dc.SetPen(wx.Pen(self.color2, 1))

        # wx.BoxSizer, wx.StaticBoxSizer
        if isinstance(sizer, wx.BoxSizer):
            # NOTE: we have to do some reverse-engineering here for
            # borders because the sizer and sizer item don't give us
            # enough information to know for sure where item
            # (allocated) boundaries are, just the boundaries of the
            # actual widgets. TODO: It would be nice to add something
            # to wx.SizerItem that would give us the full bounds, but
            # that will have to wait until 2.9...
            x, y = rect.GetPosition()
            if sizer.Orientation == wx.HORIZONTAL:
                y1 = y + rect.height
                for item in sizer.GetChildren():
                    ir = self.AdjustRect(tlw, win, item.Rect)
                    x = ir.x
                    if item.Flag & wx.LEFT:
                        x -= item.Border
                    dc.DrawLine(x, y, x, y1)
                    if item.IsSizer():
                        dc.DrawRectangleRect(ir)

            if sizer.Orientation == wx.VERTICAL:
                x1 = x + rect.width
                for item in sizer.GetChildren():
                    ir = self.AdjustRect(tlw, win, item.Rect)
                    y = ir.y
                    if item.Flag & wx.TOP:
                        y -= item.Border
                    dc.DrawLine(x, y, x1, y)
                    if item.IsSizer():
                        dc.DrawRectangleRect(ir)

        # wx.FlexGridSizer, wx.GridBagSizer
        elif isinstance(sizer, wx.FlexGridSizer):
            sizer.Layout()
            y = rect.y
            for rh in sizer.RowHeights[:-1]:
                y += rh
                dc.DrawLine(rect.x, y, rect.x+rect.width, y)
                y+= sizer.VGap
                dc.DrawLine(rect.x, y, rect.x+rect.width, y)
            x = rect.x
            for cw in sizer.ColWidths[:-1]:
                x += cw
                dc.DrawLine(x, rect.y, x, rect.y+rect.height)
                x+= sizer.HGap
                dc.DrawLine(x, rect.y, x, rect.y+rect.height)

        # wx.GridSizer
        elif isinstance(sizer, wx.GridSizer):
            # NOTE: More reverse engineering (see above.) This time we
            # need to determine what the sizer is using for row
            # heights and column widths.
            #rh = cw = 0
            #for item in sizer.GetChildren():
            #    rh = max(rh, item.Size.height)
            #    cw = max(cw, item.Size.width)
            cw = (rect.width - sizer.HGap*(sizer.Cols-1)) / sizer.Cols
            rh = (rect.height - sizer.VGap*(sizer.Rows-1)) / sizer.Rows
            y = rect.y
            for i in range(sizer.Rows-1):
                y += rh
                dc.DrawLine(rect.x, y, rect.x+rect.width, y)
                y+= sizer.VGap
                dc.DrawLine(rect.x, y, rect.x+rect.width, y)
            x = rect.x
            for i in range(sizer.Cols-1):
                x += cw
                dc.DrawLine(x, rect.y, x, rect.y+rect.height)
                x+= sizer.HGap
                dc.DrawLine(x, rect.y, x, rect.y+rect.height)

        # Anything else is probably a custom sizer, just highlight the items
        else:
            del dc
            for item in sizer.GetChildren():
                self.HighlightSizerItem(item, sizer, 1)


    def FindHighlightPos(self, tlw, pos):
        if 'wxMac' in wx.PlatformInfo:
            # We'll be using a WindowDC in this case so adjust the
            # position accordingly
            pos = tlw.ScreenToClient(pos)
            useWinDC = True
        else:
            useWinDC = False
        return pos, useWinDC


    def AdjustRect(self, tlw, win,  rect):
        pos, j = self.FindHighlightPos(tlw, win.ClientToScreen(rect.Position))
        rect.Position = pos
        return wx.RectPS(pos, rect.Size)


    def DoHighlight(self, tlw, rect, colour, useWinDC, penWidth=2):
        if useWinDC:
            dc = wx.WindowDC(tlw)
        else:
            dc = wx.ScreenDC()
        dc.SetPen(wx.Pen(colour, penWidth))
        dc.SetBrush(wx.TRANSPARENT_BRUSH)

        drawRect = wx.Rect(*rect)
        dc.DrawRectangleRect(drawRect)

        drawRect.Inflate(2,2)
        if not useWinDC:
            pos = tlw.ScreenToClient(drawRect.GetPosition())
            drawRect.SetPosition(pos)
        wx.CallLater(self.highlightTime, tlw.RefreshRect, drawRect)

        return dc


    def FlickerTLW(self, tlw):
        """
        Use a timer to alternate a TLW between shown and hidded state a
        few times.  Use to highlight a TLW since drawing and clearing an
        outline is trickier.
        """
        self.flickerCount = 0
        tlw.Hide()
        self.cl = wx.CallLater(300, self._Toggle, tlw)

    def _Toggle(self, tlw):
        if tlw.IsShown():
            tlw.Hide()
            self.cl.Restart()
        else:
            tlw.Show()
            self.flickerCount += 1
            if self.flickerCount < 4:
                self.cl.Restart()


#---------------------------------------------------------------------------
from wx.lib.embeddedimage import PyEmbeddedImage

Refresh = PyEmbeddedImage(
    "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"
    "GXlKb/xX4wAAAABJRU5ErkJggg==")

Find = PyEmbeddedImage(
    "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")

ShowSizers = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABgAAAAYCAYAAADgdz34AAAABHNCSVQICAgIfAhkiAAABChJ"
    "REFUSIm1lU1oXFUUx3/33nn3zbx5mcm0M5OZxGlNAn6XUqlRKIIWKrXqRlCLOwvitrooqKDg"
    "RhcirkSpigSECl0WBa104UKoSIoLS8TWlja1NTOTyUzz5s37uNdFWpumJSktHriry//8zj38"
    "z7nwP4dY5/7BUqn0quu62621i9cJhSjGcTzTarUOAr/dFn1sbGy/N+SdksgoQ2bh2mFBQpTz"
    "vdP1ev3AWjkya10qpe4yPbPZ3GeUecEMswQoYK4I3w+wzWiz3qgbtw0AQoHoswWfNxyYLYE2"
    "8GcVfr8IzU5gjOnfCWA5YuCvHPw0CRkLhGDjW5LeGiAFWjGcaYKyUHLAwvoeWQcgpczZjM3z"
    "A3CiD5fPAlikFXRicNy8lNJbK4das/A0VdKVJd/4oWqJZhSGVcJUeH3n5JA/NGe1OhEG/W+i"
    "KJq9LUAURe1KuVJQrrqnn4YVrXXkOE5gFCpf8NteLvdts9k8AgRXJDf0bC1ArlgsTiVJsqfX"
    "6+1K03RQLpenfd//tdvtbh0MBvdaaxe01pcGg8FFlq1wU8jNoqiUeq5Wqx3SWlutdWvTpk3v"
    "ANuBbY1G423Xdecdx7EjIyOHlVLPA6X1kl4lK6XU7rGxsU+UUkue54XlcvlL4LEVL36kUql8"
    "ns/nl6SUwejo6EGl1DNcM81/r7ihRfl8fmehUHil2Wzu1Vr3Xdc9lCTJZ4PB4DhXzAlcAOa0"
    "1koIMdntdqdKpZInhGjHcXx6ZT4FjDQajR21Wm2L7/sPJ0nyYr/ff1ZKOV8ul2eTJDnS6XSO"
    "sjwN4mp1cRw3s9lssVQq1cIwVEmSbNVa+9VqNTsyMjLh+/744uJiT3ied8BxnJcx6QMS0wkH"
    "UaEfJe6w4mc8v5AINWOS+KMgCGZWVuZ53taMlPtRaqovTRAv9LaTdQIn5y0oYzZkkaejOJ6m"
    "Xq8fk0IkCGVxci2UnsfNz1MatSDtUN7rT0xMvLa6lePj4/v8wlAfsCVUuJFMZxh1eQMqyIFF"
    "irRerx/LGGMCYa3ioV1f8el30/w4k8V178bNfMjHL3mcPREA0U1MES3ZNK2R6Z9katpgkpBU"
    "jFLovcnJRz8QF54wxgTXVoUQl9iBzz/bniQlT39JIdecQywICfEwQwFkd4KqdAmDS8gKEMLK"
    "XZTaOVImsbyOpM1QXiPkmgBAZBApeOEsS5OSjD8gJYsG6AFkhBA5C3D6+G72vudTGYXg8gYQ"
    "0J6DDB4sK67LLISjhXQ6xLm3+OXxU6RuBGxEM0sPLFkhRC5jjDmntD5D2N4jDr8LsMCV+bCQ"
    "t8Xhs0mStFYD4jhu55B/LEpx//vi/A5gieWdJLBoV+m2MeacAJ6uVqt7pZQNa+11v5MQohAE"
    "wdFut/sFcH4VY9T3/X2+7z9lre2t0mXTNP17fn7+6/V6fMfxL1klnkaQRVDaAAAAAElFTkSu"
    "QmCC")

ShowFilling = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABgAAAAYCAYAAADgdz34AAAABHNCSVQICAgIfAhkiAAAAZ5J"
    "REFUSIntlc9KAkEcxz+tu7OxLRSuGUXtIfUpukW9QJcO3nqELoLXLr2Bt47dRBAqgxJ6BqMw"
    "PJe0FEliaLNrFxV1zXatY9/TMDPf72eG3/yBf/2guaH2DrALrADtGfME8AKUgCsAdWhwX1XV"
    "bSnlMtCZEaApivrqeXJxEmBDSrl+dHQoNjf3OD9/xjSDJ5vmMre3Z1xeHi8AG/3+YcAH8GEY"
    "SbG6uoVtP2IYwQFLS2s8PT0MciYBALi5uUfTrrEsB88LDtB1C027A+h+N6cAvBUK+e6surg4"
    "6wLvvSwAlHFKu/0ZfNkBvD6AEGJmwCSvrwaNRgOAZrMZKtw0zYF3KiCXy1Eul2m1WqEAhmFQ"
    "q9VgrMg+QKVSoVqt4oU5QoCiKHQ6/vvpA2QyGdLpNPV6PRQgHo+Tz+fJZrPTAYlEgmQyiW3b"
    "oQBCCFKpFIy+b36ArusDQ1j1vCM18B3TaDQaOrgvy7Jgyg7mgflisQiA4zihwmOxGKVSCUDv"
    "ZTFOO/mL5zoSiby6rnsFHMDoDk6llA6//HBc1+1/OP8Kpi8497f1tG0HzQAAAABJRU5ErkJg"
    "gg==")

Icon = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAAABHNCSVQICAgIfAhkiAAAALhJ"
    "REFUWIXtl80SgyAMhHeR99Y+eJseLAdHbJM0TjiwN2dI+MgfQpYFmSqpu0+AEQDqrwXyeorH"
    "McvCEIBdm3F7/fr0FKgBRFaIrHkAdykdQFmEGm2HL233BAIAYmxYEqjePo9SBYBvBKppclDz"
    "prMcqAhbAtknJx+3AKRHgGhnv4iApQY+jtSWpOY27BnifNt5uyk9BekAoZNwl21yDBSBi/63"
    "yOMiLAXaf8AuwP9n94vzaTYBsgHeht4lXXmb7yQAAAAASUVORK5CYII=")

CollapseTree = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABgAAAAYCAYAAADgdz34AAAABHNCSVQICAgIfAhkiAAAAf9J"
    "REFUSIm9lD9r20AYhx9Fxe5SKDIynVJodFMaChkC6eBCJ+WWdulSb1o6RR8gH6AfQF5aSksX"
    "aQ0eajwVkinO1MFk0YXg0uLSISZDoQhcddAf5MR241buD4670/tyD+/vXh0sWdqUb1vAQ2Bj"
    "Suwb0E7Xx38DaAKPgTuAnJLfSSEAn4DWIoAm8ByQruti2zYAlmUBoJSi2+3ieV4R1v0TJANs"
    "AS8Ap9PpYFkWUSSuJFcqIUopAKSUAO+A18yxS0/nZ8AD13WFbdtEkWB9Hep1ODqC0QgMA8bj"
    "GqYJhmGgaRq9Xm8I3AY+zgKspPMGIIuHZ6qn4/Q02UeRIIpEZqEkua+ZWpkXLEM3ipvEe9C0"
    "ad2bqN+P89zr6P9WoJRidVXQ78e55/U09h1YW5vMTTWcB8i66B7wq1arie1ti/G4hmEknfNl"
    "BD8Kh1cqIbp+ThAEVKtVBoNBCziZBfjX/wDgEHg0C7D0O8gs+grcAm76vi80TcM04eJCYZqg"
    "6+ecnR0TBAGu6+L7PlJKhAgJQ+6SvF/vrwNsAm+BD0B8eTQajXztOMT7HnFrL48fpGNCizzX"
    "Q5IXdDfdN/Y92Hna5s2rJ+y+zPPm3skiOoCkip+f23Fr70o1pWgCkoGKkKV3URnKqyjaxTKs"
    "omCX4+SQ0pS1aew4xJub5VZwGZQdfv83yOfTR/iA1xwAAAAASUVORK5CYII=")

ExpandTree = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABgAAAAYCAYAAADgdz34AAAABHNCSVQICAgIfAhkiAAAAepJ"
    "REFUSIm1lDFr20AYhp+ri+iSxUUhW4dIU9wlQ4YOKnRSj4K7Z9NSCFg/ID+gP0BZWkpLF3kN"
    "Giq0pkOpMxWSTFIIFIqHQCFbUQnXQTpFslXXptYLx90nvdzD9913Bx1LtHzbA54Aj1v+nQFf"
    "yvXpMoD7M/E+8AzYAmSLP66BbSBcBTACXED6vo/rugBYlgVAlmUkSSKDIND+rXJeCNEl2gNe"
    "AV4cx1iWRZ7bc2bDSMmyDAApJcAH4C0LytUr5wPA8n3fdl2XPLfZ2YHNTbi+vjPf3j7ENKHf"
    "7yOEYDKZTIHfwNe/Ae7V0pX1zbUGA8FgILi8LOI8t8lzW5dQ0t4Mc4DO1ADoA724ACEEQtx1"
    "8XBYZDLrXQnQhRoA3SEAUaSIItWIz89Vm3e6DOAMiJMkwTBSALa3i6Gl14aRYhgpSZLgOA7A"
    "t0WA/70HAJ+Bp//KoDPpi/YD2AAehGFoCyEwTbi5yTBN6PV+cnV1yng8xvd9wjBESoltp6Qp"
    "jyjer4/LAPeB98AnQM0Ox3GqteehjgPU0WH1/6QcDa3yXE8pDnRUxs5xAM9fRrx7M2T0uvIt"
    "PJNVdAJFFr++R+rocC6btagB0aA6pPMuWoeqLOrlootSUSuX51WQtUm3qfI81O7uejOYBenN"
    "X/wBVz/ONKbGYPkAAAAASUVORK5CYII=")

HighlightItem = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABgAAAAYCAYAAADgdz34AAAABHNCSVQICAgIfAhkiAAAAQZJ"
    "REFUSInFlTGSgjAUhv8XuIRl9ga0XgmuoI5abwfH8Ai22GlJZ0otuQD5t3DZ1V1CwgzgP8OQ"
    "QCZfkv+9FxEVYUrFbYO2oW+wqEiGAtRzhyRIQh9eH+RXAMBmvfIuohcwheLnTnZ6vM3NjAaQ"
    "1mTahvrAHwCzj+BJVI83sesHAMjRM3OVgNkFm/WK292+EzKvB86zr5Lu76b2AubdAbqMda0+"
    "UOIqFdY2lKMHYGrw06DL3Tbrxzmi/Iq0JNLyO/Pxm/Uze/BXVRIUKajvKM6AXuh/kfjeHTC7"
    "TAdw1RfahmlJFOewgtjvQY/0QgeNe3MUOVQsw2/OwQBRkQy5Op2lYixN7sEXVhRd4PXVHvwA"
    "AAAASUVORK5CYII=")

EvtWatcher = PyEmbeddedImage(
    "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")
