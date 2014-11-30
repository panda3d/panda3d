# This file was created automatically by SWIG 1.3.29.
# Don't modify this file, modify the SWIG interface instead.

"""
The wx.aui module is an Advanced User Interface library that aims to
implement "cutting-edge" interface usability and design features so
developers can quickly and easily create beautiful and usable
application interfaces.

**Vision and Design Principles**

wx.aui attempts to encapsulate the following aspects of the user
interface:

  * Frame Management: Frame management provides the means to open,
    move and hide common controls that are needed to interact with the
    document, and allow these configurations to be saved into
    different perspectives and loaded at a later time.

  * Toolbars: Toolbars are a specialized subset of the frame
    management system and should behave similarly to other docked
    components. However, they also require additional functionality,
    such as "spring-loaded" rebar support, "chevron" buttons and
    end-user customizability.

  * Modeless Controls: Modeless controls expose a tool palette or set
    of options that float above the application content while allowing
    it to be accessed. Usually accessed by the toolbar, these controls
    disappear when an option is selected, but may also be "torn off"
    the toolbar into a floating frame of their own.

  * Look and Feel: Look and feel encompasses the way controls are
    drawn, both when shown statically as well as when they are being
    moved. This aspect of user interface design incorporates "special
    effects" such as transparent window dragging as well as frame
    animation.

**wx.aui adheres to the following principles**

  - Use native floating frames to obtain a native look and feel for
    all platforms;

  - Use existing wxPython code where possible, such as sizer
    implementation for frame management;

  - Use standard wxPython coding conventions.


**Usage**

The following example shows a simple implementation that utilizes
`wx.aui.AuiManager` to manage three text controls in a frame window::

    import wx
    import wx.aui

    class MyFrame(wx.Frame):

        def __init__(self, parent, id=-1, title='wx.aui Test',
                     pos=wx.DefaultPosition, size=(800, 600),
                     style=wx.DEFAULT_FRAME_STYLE):
            wx.Frame.__init__(self, parent, id, title, pos, size, style)

            self._mgr = wx.aui.AuiManager(self)

            # create several text controls
            text1 = wx.TextCtrl(self, -1, 'Pane 1 - sample text',
                                wx.DefaultPosition, wx.Size(200,150),
                                wx.NO_BORDER | wx.TE_MULTILINE)

            text2 = wx.TextCtrl(self, -1, 'Pane 2 - sample text',
                                wx.DefaultPosition, wx.Size(200,150),
                                wx.NO_BORDER | wx.TE_MULTILINE)

            text3 = wx.TextCtrl(self, -1, 'Main content window',
                                wx.DefaultPosition, wx.Size(200,150),
                                wx.NO_BORDER | wx.TE_MULTILINE)

            # add the panes to the manager
            self._mgr.AddPane(text1, wx.LEFT, 'Pane Number One')
            self._mgr.AddPane(text2, wx.BOTTOM, 'Pane Number Two')
            self._mgr.AddPane(text3, wx.CENTER)

            # tell the manager to 'commit' all the changes just made
            self._mgr.Update()

            self.Bind(wx.EVT_CLOSE, self.OnClose)


        def OnClose(self, event):
            # deinitialize the frame manager
            self._mgr.UnInit()
            # delete the frame
            self.Destroy()


    app = wx.App()
    frame = MyFrame(None)
    frame.Show()
    app.MainLoop()

"""

import _aui
import new
new_instancemethod = new.instancemethod
def _swig_setattr_nondynamic(self,class_type,name,value,static=1):
    if (name == "thisown"): return self.this.own(value)
    if (name == "this"):
        if type(value).__name__ == 'PySwigObject':
            self.__dict__[name] = value
            return
    method = class_type.__swig_setmethods__.get(name,None)
    if method: return method(self,value)
    if (not static) or hasattr(self,name):
        self.__dict__[name] = value
    else:
        raise AttributeError("You cannot add attributes to %s" % self)

def _swig_setattr(self,class_type,name,value):
    return _swig_setattr_nondynamic(self,class_type,name,value,0)

def _swig_getattr(self,class_type,name):
    if (name == "thisown"): return self.this.own()
    method = class_type.__swig_getmethods__.get(name,None)
    if method: return method(self)
    raise AttributeError,name

def _swig_repr(self):
    try: strthis = "proxy of " + self.this.__repr__()
    except: strthis = ""
    return "<%s.%s; %s >" % (self.__class__.__module__, self.__class__.__name__, strthis,)

import types
try:
    _object = types.ObjectType
    _newclass = 1
except AttributeError:
    class _object : pass
    _newclass = 0
del types


def _swig_setattr_nondynamic_method(set):
    def set_attr(self,name,value):
        if (name == "thisown"): return self.this.own(value)
        if hasattr(self,name) or (name == "this"):
            set(self,name,value)
        else:
            raise AttributeError("You cannot add attributes to %s" % self)
    return set_attr


import _core
import _windows
wx = _core 
__docfilter__ = wx.__DocFilter(globals()) 
AUI_DOCK_NONE = _aui.AUI_DOCK_NONE
AUI_DOCK_TOP = _aui.AUI_DOCK_TOP
AUI_DOCK_RIGHT = _aui.AUI_DOCK_RIGHT
AUI_DOCK_BOTTOM = _aui.AUI_DOCK_BOTTOM
AUI_DOCK_LEFT = _aui.AUI_DOCK_LEFT
AUI_DOCK_CENTER = _aui.AUI_DOCK_CENTER
AUI_DOCK_CENTRE = _aui.AUI_DOCK_CENTRE
AUI_MGR_ALLOW_FLOATING = _aui.AUI_MGR_ALLOW_FLOATING
AUI_MGR_ALLOW_ACTIVE_PANE = _aui.AUI_MGR_ALLOW_ACTIVE_PANE
AUI_MGR_TRANSPARENT_DRAG = _aui.AUI_MGR_TRANSPARENT_DRAG
AUI_MGR_TRANSPARENT_HINT = _aui.AUI_MGR_TRANSPARENT_HINT
AUI_MGR_VENETIAN_BLINDS_HINT = _aui.AUI_MGR_VENETIAN_BLINDS_HINT
AUI_MGR_RECTANGLE_HINT = _aui.AUI_MGR_RECTANGLE_HINT
AUI_MGR_HINT_FADE = _aui.AUI_MGR_HINT_FADE
AUI_MGR_NO_VENETIAN_BLINDS_FADE = _aui.AUI_MGR_NO_VENETIAN_BLINDS_FADE
AUI_MGR_LIVE_RESIZE = _aui.AUI_MGR_LIVE_RESIZE
AUI_MGR_DEFAULT = _aui.AUI_MGR_DEFAULT
AUI_DOCKART_SASH_SIZE = _aui.AUI_DOCKART_SASH_SIZE
AUI_DOCKART_CAPTION_SIZE = _aui.AUI_DOCKART_CAPTION_SIZE
AUI_DOCKART_GRIPPER_SIZE = _aui.AUI_DOCKART_GRIPPER_SIZE
AUI_DOCKART_PANE_BORDER_SIZE = _aui.AUI_DOCKART_PANE_BORDER_SIZE
AUI_DOCKART_PANE_BUTTON_SIZE = _aui.AUI_DOCKART_PANE_BUTTON_SIZE
AUI_DOCKART_BACKGROUND_COLOUR = _aui.AUI_DOCKART_BACKGROUND_COLOUR
AUI_DOCKART_SASH_COLOUR = _aui.AUI_DOCKART_SASH_COLOUR
AUI_DOCKART_ACTIVE_CAPTION_COLOUR = _aui.AUI_DOCKART_ACTIVE_CAPTION_COLOUR
AUI_DOCKART_ACTIVE_CAPTION_GRADIENT_COLOUR = _aui.AUI_DOCKART_ACTIVE_CAPTION_GRADIENT_COLOUR
AUI_DOCKART_INACTIVE_CAPTION_COLOUR = _aui.AUI_DOCKART_INACTIVE_CAPTION_COLOUR
AUI_DOCKART_INACTIVE_CAPTION_GRADIENT_COLOUR = _aui.AUI_DOCKART_INACTIVE_CAPTION_GRADIENT_COLOUR
AUI_DOCKART_ACTIVE_CAPTION_TEXT_COLOUR = _aui.AUI_DOCKART_ACTIVE_CAPTION_TEXT_COLOUR
AUI_DOCKART_INACTIVE_CAPTION_TEXT_COLOUR = _aui.AUI_DOCKART_INACTIVE_CAPTION_TEXT_COLOUR
AUI_DOCKART_BORDER_COLOUR = _aui.AUI_DOCKART_BORDER_COLOUR
AUI_DOCKART_GRIPPER_COLOUR = _aui.AUI_DOCKART_GRIPPER_COLOUR
AUI_DOCKART_CAPTION_FONT = _aui.AUI_DOCKART_CAPTION_FONT
AUI_DOCKART_GRADIENT_TYPE = _aui.AUI_DOCKART_GRADIENT_TYPE
AUI_GRADIENT_NONE = _aui.AUI_GRADIENT_NONE
AUI_GRADIENT_VERTICAL = _aui.AUI_GRADIENT_VERTICAL
AUI_GRADIENT_HORIZONTAL = _aui.AUI_GRADIENT_HORIZONTAL
AUI_BUTTON_STATE_NORMAL = _aui.AUI_BUTTON_STATE_NORMAL
AUI_BUTTON_STATE_HOVER = _aui.AUI_BUTTON_STATE_HOVER
AUI_BUTTON_STATE_PRESSED = _aui.AUI_BUTTON_STATE_PRESSED
AUI_BUTTON_STATE_DISABLED = _aui.AUI_BUTTON_STATE_DISABLED
AUI_BUTTON_STATE_HIDDEN = _aui.AUI_BUTTON_STATE_HIDDEN
AUI_BUTTON_STATE_CHECKED = _aui.AUI_BUTTON_STATE_CHECKED
AUI_BUTTON_CLOSE = _aui.AUI_BUTTON_CLOSE
AUI_BUTTON_MAXIMIZE_RESTORE = _aui.AUI_BUTTON_MAXIMIZE_RESTORE
AUI_BUTTON_MINIMIZE = _aui.AUI_BUTTON_MINIMIZE
AUI_BUTTON_PIN = _aui.AUI_BUTTON_PIN
AUI_BUTTON_OPTIONS = _aui.AUI_BUTTON_OPTIONS
AUI_BUTTON_WINDOWLIST = _aui.AUI_BUTTON_WINDOWLIST
AUI_BUTTON_LEFT = _aui.AUI_BUTTON_LEFT
AUI_BUTTON_RIGHT = _aui.AUI_BUTTON_RIGHT
AUI_BUTTON_UP = _aui.AUI_BUTTON_UP
AUI_BUTTON_DOWN = _aui.AUI_BUTTON_DOWN
AUI_BUTTON_CUSTOM1 = _aui.AUI_BUTTON_CUSTOM1
AUI_BUTTON_CUSTOM2 = _aui.AUI_BUTTON_CUSTOM2
AUI_BUTTON_CUSTOM3 = _aui.AUI_BUTTON_CUSTOM3
AUI_INSERT_PANE = _aui.AUI_INSERT_PANE
AUI_INSERT_ROW = _aui.AUI_INSERT_ROW
AUI_INSERT_DOCK = _aui.AUI_INSERT_DOCK
class AuiPaneInfo(object):
    """Proxy of C++ AuiPaneInfo class"""
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    def __init__(self, *args, **kwargs): 
        """__init__(self) -> AuiPaneInfo"""
        _aui.AuiPaneInfo_swiginit(self,_aui.new_AuiPaneInfo(*args, **kwargs))
    __swig_destroy__ = _aui.delete_AuiPaneInfo
    __del__ = lambda self : None;
    def SafeSet(*args, **kwargs):
        """SafeSet(self, AuiPaneInfo source)"""
        return _aui.AuiPaneInfo_SafeSet(*args, **kwargs)

    def IsOk(*args, **kwargs):
        """IsOk(self) -> bool"""
        return _aui.AuiPaneInfo_IsOk(*args, **kwargs)

    def IsFixed(*args, **kwargs):
        """IsFixed(self) -> bool"""
        return _aui.AuiPaneInfo_IsFixed(*args, **kwargs)

    def IsResizable(*args, **kwargs):
        """IsResizable(self) -> bool"""
        return _aui.AuiPaneInfo_IsResizable(*args, **kwargs)

    def IsShown(*args, **kwargs):
        """IsShown(self) -> bool"""
        return _aui.AuiPaneInfo_IsShown(*args, **kwargs)

    def IsFloating(*args, **kwargs):
        """IsFloating(self) -> bool"""
        return _aui.AuiPaneInfo_IsFloating(*args, **kwargs)

    def IsDocked(*args, **kwargs):
        """IsDocked(self) -> bool"""
        return _aui.AuiPaneInfo_IsDocked(*args, **kwargs)

    def IsToolbar(*args, **kwargs):
        """IsToolbar(self) -> bool"""
        return _aui.AuiPaneInfo_IsToolbar(*args, **kwargs)

    def IsTopDockable(*args, **kwargs):
        """IsTopDockable(self) -> bool"""
        return _aui.AuiPaneInfo_IsTopDockable(*args, **kwargs)

    def IsBottomDockable(*args, **kwargs):
        """IsBottomDockable(self) -> bool"""
        return _aui.AuiPaneInfo_IsBottomDockable(*args, **kwargs)

    def IsLeftDockable(*args, **kwargs):
        """IsLeftDockable(self) -> bool"""
        return _aui.AuiPaneInfo_IsLeftDockable(*args, **kwargs)

    def IsRightDockable(*args, **kwargs):
        """IsRightDockable(self) -> bool"""
        return _aui.AuiPaneInfo_IsRightDockable(*args, **kwargs)

    def IsFloatable(*args, **kwargs):
        """IsFloatable(self) -> bool"""
        return _aui.AuiPaneInfo_IsFloatable(*args, **kwargs)

    def IsMovable(*args, **kwargs):
        """IsMovable(self) -> bool"""
        return _aui.AuiPaneInfo_IsMovable(*args, **kwargs)

    def IsDestroyOnClose(*args, **kwargs):
        """IsDestroyOnClose(self) -> bool"""
        return _aui.AuiPaneInfo_IsDestroyOnClose(*args, **kwargs)

    def IsMaximized(*args, **kwargs):
        """IsMaximized(self) -> bool"""
        return _aui.AuiPaneInfo_IsMaximized(*args, **kwargs)

    def HasCaption(*args, **kwargs):
        """HasCaption(self) -> bool"""
        return _aui.AuiPaneInfo_HasCaption(*args, **kwargs)

    def HasGripper(*args, **kwargs):
        """HasGripper(self) -> bool"""
        return _aui.AuiPaneInfo_HasGripper(*args, **kwargs)

    def HasBorder(*args, **kwargs):
        """HasBorder(self) -> bool"""
        return _aui.AuiPaneInfo_HasBorder(*args, **kwargs)

    def HasCloseButton(*args, **kwargs):
        """HasCloseButton(self) -> bool"""
        return _aui.AuiPaneInfo_HasCloseButton(*args, **kwargs)

    def HasMaximizeButton(*args, **kwargs):
        """HasMaximizeButton(self) -> bool"""
        return _aui.AuiPaneInfo_HasMaximizeButton(*args, **kwargs)

    def HasMinimizeButton(*args, **kwargs):
        """HasMinimizeButton(self) -> bool"""
        return _aui.AuiPaneInfo_HasMinimizeButton(*args, **kwargs)

    def HasPinButton(*args, **kwargs):
        """HasPinButton(self) -> bool"""
        return _aui.AuiPaneInfo_HasPinButton(*args, **kwargs)

    def HasGripperTop(*args, **kwargs):
        """HasGripperTop(self) -> bool"""
        return _aui.AuiPaneInfo_HasGripperTop(*args, **kwargs)

    def Window(*args, **kwargs):
        """Window(self, Window w) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_Window(*args, **kwargs)

    def Name(*args, **kwargs):
        """Name(self, String n) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_Name(*args, **kwargs)

    def Caption(*args, **kwargs):
        """Caption(self, String c) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_Caption(*args, **kwargs)

    def Left(*args, **kwargs):
        """Left(self) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_Left(*args, **kwargs)

    def Right(*args, **kwargs):
        """Right(self) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_Right(*args, **kwargs)

    def Top(*args, **kwargs):
        """Top(self) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_Top(*args, **kwargs)

    def Bottom(*args, **kwargs):
        """Bottom(self) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_Bottom(*args, **kwargs)

    def Center(*args, **kwargs):
        """Center(self) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_Center(*args, **kwargs)

    def Centre(*args, **kwargs):
        """Centre(self) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_Centre(*args, **kwargs)

    def Direction(*args, **kwargs):
        """Direction(self, int direction) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_Direction(*args, **kwargs)

    def Layer(*args, **kwargs):
        """Layer(self, int layer) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_Layer(*args, **kwargs)

    def Row(*args, **kwargs):
        """Row(self, int row) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_Row(*args, **kwargs)

    def Position(*args, **kwargs):
        """Position(self, int pos) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_Position(*args, **kwargs)

    def BestSize(*args, **kwargs):
        """BestSize(self, Size size) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_BestSize(*args, **kwargs)

    def MinSize(*args, **kwargs):
        """MinSize(self, Size size) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_MinSize(*args, **kwargs)

    def MaxSize(*args, **kwargs):
        """MaxSize(self, Size size) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_MaxSize(*args, **kwargs)

    def FloatingPosition(*args, **kwargs):
        """FloatingPosition(self, Point pos) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_FloatingPosition(*args, **kwargs)

    def FloatingSize(*args, **kwargs):
        """FloatingSize(self, Size size) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_FloatingSize(*args, **kwargs)

    def Fixed(*args, **kwargs):
        """Fixed(self) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_Fixed(*args, **kwargs)

    def Resizable(*args, **kwargs):
        """Resizable(self, bool resizable=True) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_Resizable(*args, **kwargs)

    def Dock(*args, **kwargs):
        """Dock(self) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_Dock(*args, **kwargs)

    def Float(*args, **kwargs):
        """Float(self) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_Float(*args, **kwargs)

    def Hide(*args, **kwargs):
        """Hide(self) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_Hide(*args, **kwargs)

    def Show(*args, **kwargs):
        """Show(self, bool show=True) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_Show(*args, **kwargs)

    def CaptionVisible(*args, **kwargs):
        """CaptionVisible(self, bool visible=True) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_CaptionVisible(*args, **kwargs)

    def Maximize(*args, **kwargs):
        """Maximize(self) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_Maximize(*args, **kwargs)

    def Restore(*args, **kwargs):
        """Restore(self) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_Restore(*args, **kwargs)

    def PaneBorder(*args, **kwargs):
        """PaneBorder(self, bool visible=True) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_PaneBorder(*args, **kwargs)

    def Gripper(*args, **kwargs):
        """Gripper(self, bool visible=True) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_Gripper(*args, **kwargs)

    def GripperTop(*args, **kwargs):
        """GripperTop(self, bool attop=True) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_GripperTop(*args, **kwargs)

    def CloseButton(*args, **kwargs):
        """CloseButton(self, bool visible=True) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_CloseButton(*args, **kwargs)

    def MaximizeButton(*args, **kwargs):
        """MaximizeButton(self, bool visible=True) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_MaximizeButton(*args, **kwargs)

    def MinimizeButton(*args, **kwargs):
        """MinimizeButton(self, bool visible=True) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_MinimizeButton(*args, **kwargs)

    def PinButton(*args, **kwargs):
        """PinButton(self, bool visible=True) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_PinButton(*args, **kwargs)

    def DestroyOnClose(*args, **kwargs):
        """DestroyOnClose(self, bool b=True) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_DestroyOnClose(*args, **kwargs)

    def TopDockable(*args, **kwargs):
        """TopDockable(self, bool b=True) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_TopDockable(*args, **kwargs)

    def BottomDockable(*args, **kwargs):
        """BottomDockable(self, bool b=True) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_BottomDockable(*args, **kwargs)

    def LeftDockable(*args, **kwargs):
        """LeftDockable(self, bool b=True) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_LeftDockable(*args, **kwargs)

    def RightDockable(*args, **kwargs):
        """RightDockable(self, bool b=True) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_RightDockable(*args, **kwargs)

    def Floatable(*args, **kwargs):
        """Floatable(self, bool b=True) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_Floatable(*args, **kwargs)

    def Movable(*args, **kwargs):
        """Movable(self, bool b=True) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_Movable(*args, **kwargs)

    def DockFixed(*args, **kwargs):
        """DockFixed(self, bool b=True) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_DockFixed(*args, **kwargs)

    def Dockable(*args, **kwargs):
        """Dockable(self, bool b=True) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_Dockable(*args, **kwargs)

    def DefaultPane(*args, **kwargs):
        """DefaultPane(self) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_DefaultPane(*args, **kwargs)

    def CentrePane(*args, **kwargs):
        """CentrePane(self) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_CentrePane(*args, **kwargs)

    def CenterPane(*args, **kwargs):
        """CenterPane(self) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_CenterPane(*args, **kwargs)

    def ToolbarPane(*args, **kwargs):
        """ToolbarPane(self) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_ToolbarPane(*args, **kwargs)

    def SetFlag(*args, **kwargs):
        """SetFlag(self, int flag, bool option_state) -> AuiPaneInfo"""
        return _aui.AuiPaneInfo_SetFlag(*args, **kwargs)

    def HasFlag(*args, **kwargs):
        """HasFlag(self, int flag) -> bool"""
        return _aui.AuiPaneInfo_HasFlag(*args, **kwargs)

    optionFloating = _aui.AuiPaneInfo_optionFloating
    optionHidden = _aui.AuiPaneInfo_optionHidden
    optionLeftDockable = _aui.AuiPaneInfo_optionLeftDockable
    optionRightDockable = _aui.AuiPaneInfo_optionRightDockable
    optionTopDockable = _aui.AuiPaneInfo_optionTopDockable
    optionBottomDockable = _aui.AuiPaneInfo_optionBottomDockable
    optionFloatable = _aui.AuiPaneInfo_optionFloatable
    optionMovable = _aui.AuiPaneInfo_optionMovable
    optionResizable = _aui.AuiPaneInfo_optionResizable
    optionPaneBorder = _aui.AuiPaneInfo_optionPaneBorder
    optionCaption = _aui.AuiPaneInfo_optionCaption
    optionGripper = _aui.AuiPaneInfo_optionGripper
    optionDestroyOnClose = _aui.AuiPaneInfo_optionDestroyOnClose
    optionToolbar = _aui.AuiPaneInfo_optionToolbar
    optionActive = _aui.AuiPaneInfo_optionActive
    optionGripperTop = _aui.AuiPaneInfo_optionGripperTop
    optionMaximized = _aui.AuiPaneInfo_optionMaximized
    optionDockFixed = _aui.AuiPaneInfo_optionDockFixed
    buttonClose = _aui.AuiPaneInfo_buttonClose
    buttonMaximize = _aui.AuiPaneInfo_buttonMaximize
    buttonMinimize = _aui.AuiPaneInfo_buttonMinimize
    buttonPin = _aui.AuiPaneInfo_buttonPin
    buttonCustom1 = _aui.AuiPaneInfo_buttonCustom1
    buttonCustom2 = _aui.AuiPaneInfo_buttonCustom2
    buttonCustom3 = _aui.AuiPaneInfo_buttonCustom3
    savedHiddenState = _aui.AuiPaneInfo_savedHiddenState
    actionPane = _aui.AuiPaneInfo_actionPane
    name = property(_aui.AuiPaneInfo_name_get, _aui.AuiPaneInfo_name_set)
    caption = property(_aui.AuiPaneInfo_caption_get, _aui.AuiPaneInfo_caption_set)
    window = property(_aui.AuiPaneInfo_window_get, _aui.AuiPaneInfo_window_set)
    frame = property(_aui.AuiPaneInfo_frame_get, _aui.AuiPaneInfo_frame_set)
    state = property(_aui.AuiPaneInfo_state_get, _aui.AuiPaneInfo_state_set)
    dock_direction = property(_aui.AuiPaneInfo_dock_direction_get, _aui.AuiPaneInfo_dock_direction_set)
    dock_layer = property(_aui.AuiPaneInfo_dock_layer_get, _aui.AuiPaneInfo_dock_layer_set)
    dock_row = property(_aui.AuiPaneInfo_dock_row_get, _aui.AuiPaneInfo_dock_row_set)
    dock_pos = property(_aui.AuiPaneInfo_dock_pos_get, _aui.AuiPaneInfo_dock_pos_set)
    best_size = property(_aui.AuiPaneInfo_best_size_get, _aui.AuiPaneInfo_best_size_set)
    min_size = property(_aui.AuiPaneInfo_min_size_get, _aui.AuiPaneInfo_min_size_set)
    max_size = property(_aui.AuiPaneInfo_max_size_get, _aui.AuiPaneInfo_max_size_set)
    floating_pos = property(_aui.AuiPaneInfo_floating_pos_get, _aui.AuiPaneInfo_floating_pos_set)
    floating_size = property(_aui.AuiPaneInfo_floating_size_get, _aui.AuiPaneInfo_floating_size_set)
    dock_proportion = property(_aui.AuiPaneInfo_dock_proportion_get, _aui.AuiPaneInfo_dock_proportion_set)
    buttons = property(_aui.AuiPaneInfo_buttons_get, _aui.AuiPaneInfo_buttons_set)
    rect = property(_aui.AuiPaneInfo_rect_get, _aui.AuiPaneInfo_rect_set)
_aui.AuiPaneInfo_swigregister(AuiPaneInfo)
cvar = _aui.cvar

class AuiManager(_core.EvtHandler):
    """Proxy of C++ AuiManager class"""
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    def __init__(self, *args, **kwargs): 
        """__init__(self, Window managed_wnd=None, int flags=AUI_MGR_DEFAULT) -> AuiManager"""
        _aui.AuiManager_swiginit(self,_aui.new_AuiManager(*args, **kwargs))
    __swig_destroy__ = _aui.delete_AuiManager
    __del__ = lambda self : None;
    def UnInit(*args, **kwargs):
        """UnInit(self)"""
        return _aui.AuiManager_UnInit(*args, **kwargs)

    def SetFlags(*args, **kwargs):
        """SetFlags(self, int flags)"""
        return _aui.AuiManager_SetFlags(*args, **kwargs)

    def GetFlags(*args, **kwargs):
        """GetFlags(self) -> int"""
        return _aui.AuiManager_GetFlags(*args, **kwargs)

    def SetManagedWindow(*args, **kwargs):
        """SetManagedWindow(self, Window managed_wnd)"""
        return _aui.AuiManager_SetManagedWindow(*args, **kwargs)

    def GetManagedWindow(*args, **kwargs):
        """GetManagedWindow(self) -> Window"""
        return _aui.AuiManager_GetManagedWindow(*args, **kwargs)

    def GetManager(*args, **kwargs):
        """GetManager(Window window) -> AuiManager"""
        return _aui.AuiManager_GetManager(*args, **kwargs)

    GetManager = staticmethod(GetManager)
    def SetArtProvider(*args, **kwargs):
        """SetArtProvider(self, AuiDockArt art_provider)"""
        return _aui.AuiManager_SetArtProvider(*args, **kwargs)

    def GetArtProvider(*args, **kwargs):
        """GetArtProvider(self) -> AuiDockArt"""
        return _aui.AuiManager_GetArtProvider(*args, **kwargs)

    def _GetPaneByWidget(*args, **kwargs):
        """_GetPaneByWidget(self, Window window) -> AuiPaneInfo"""
        return _aui.AuiManager__GetPaneByWidget(*args, **kwargs)

    def _GetPaneByName(*args, **kwargs):
        """_GetPaneByName(self, String name) -> AuiPaneInfo"""
        return _aui.AuiManager__GetPaneByName(*args, **kwargs)

    def GetAllPanes(*args, **kwargs):
        """GetAllPanes(self) -> wxAuiPaneInfoArray"""
        return _aui.AuiManager_GetAllPanes(*args, **kwargs)

    def _AddPane1(*args, **kwargs):
        """_AddPane1(self, Window window, AuiPaneInfo pane_info) -> bool"""
        return _aui.AuiManager__AddPane1(*args, **kwargs)

    def AddPane(*args, **kwargs):
        """AddPane(self, Window window, AuiPaneInfo pane_info, Point drop_pos) -> bool"""
        return _aui.AuiManager_AddPane(*args, **kwargs)

    def _AddPane2(*args, **kwargs):
        """_AddPane2(self, Window window, int direction=LEFT, String caption=wxEmptyString) -> bool"""
        return _aui.AuiManager__AddPane2(*args, **kwargs)

    def InsertPane(*args, **kwargs):
        """InsertPane(self, Window window, AuiPaneInfo insert_location, int insert_level=AUI_INSERT_PANE) -> bool"""
        return _aui.AuiManager_InsertPane(*args, **kwargs)

    def DetachPane(*args, **kwargs):
        """DetachPane(self, Window window) -> bool"""
        return _aui.AuiManager_DetachPane(*args, **kwargs)

    def Update(*args, **kwargs):
        """Update(self)"""
        return _aui.AuiManager_Update(*args, **kwargs)

    def SavePaneInfo(*args, **kwargs):
        """SavePaneInfo(self, AuiPaneInfo pane) -> String"""
        return _aui.AuiManager_SavePaneInfo(*args, **kwargs)

    def LoadPaneInfo(*args, **kwargs):
        """LoadPaneInfo(self, String pane_part, AuiPaneInfo pane)"""
        return _aui.AuiManager_LoadPaneInfo(*args, **kwargs)

    def SavePerspective(*args, **kwargs):
        """SavePerspective(self) -> String"""
        return _aui.AuiManager_SavePerspective(*args, **kwargs)

    def LoadPerspective(*args, **kwargs):
        """LoadPerspective(self, String perspective, bool update=True) -> bool"""
        return _aui.AuiManager_LoadPerspective(*args, **kwargs)

    def SetDockSizeConstraint(*args, **kwargs):
        """SetDockSizeConstraint(self, double width_pct, double height_pct)"""
        return _aui.AuiManager_SetDockSizeConstraint(*args, **kwargs)

    def GetDockSizeConstraint(*args, **kwargs):
        """GetDockSizeConstraint(self, double width_pct, double height_pct)"""
        return _aui.AuiManager_GetDockSizeConstraint(*args, **kwargs)

    def ClosePane(*args, **kwargs):
        """ClosePane(self, AuiPaneInfo pane_info)"""
        return _aui.AuiManager_ClosePane(*args, **kwargs)

    def MaximizePane(*args, **kwargs):
        """MaximizePane(self, AuiPaneInfo pane_info)"""
        return _aui.AuiManager_MaximizePane(*args, **kwargs)

    def RestorePane(*args, **kwargs):
        """RestorePane(self, AuiPaneInfo pane_info)"""
        return _aui.AuiManager_RestorePane(*args, **kwargs)

    def RestoreMaximizedPane(*args, **kwargs):
        """RestoreMaximizedPane(self)"""
        return _aui.AuiManager_RestoreMaximizedPane(*args, **kwargs)

    def CreateFloatingFrame(*args, **kwargs):
        """CreateFloatingFrame(self, Window parent, AuiPaneInfo p) -> AuiFloatingFrame"""
        return _aui.AuiManager_CreateFloatingFrame(*args, **kwargs)

    def StartPaneDrag(*args, **kwargs):
        """StartPaneDrag(self, Window pane_window, Point offset)"""
        return _aui.AuiManager_StartPaneDrag(*args, **kwargs)

    def CalculateHintRect(*args, **kwargs):
        """CalculateHintRect(self, Window pane_window, Point pt, Point offset) -> Rect"""
        return _aui.AuiManager_CalculateHintRect(*args, **kwargs)

    def DrawHintRect(*args, **kwargs):
        """DrawHintRect(self, Window pane_window, Point pt, Point offset)"""
        return _aui.AuiManager_DrawHintRect(*args, **kwargs)

    def ShowHint(*args, **kwargs):
        """ShowHint(self, Rect rect)"""
        return _aui.AuiManager_ShowHint(*args, **kwargs)

    def HideHint(*args, **kwargs):
        """HideHint(self)"""
        return _aui.AuiManager_HideHint(*args, **kwargs)

    def OnRender(*args, **kwargs):
        """OnRender(self, AuiManagerEvent evt)"""
        return _aui.AuiManager_OnRender(*args, **kwargs)

    def OnPaneButton(*args, **kwargs):
        """OnPaneButton(self, AuiManagerEvent evt)"""
        return _aui.AuiManager_OnPaneButton(*args, **kwargs)

    def GetPane(self, item):
        """
        GetPane(self, window_or_info item) -> PaneInfo

        GetPane is used to search for a `PaneInfo` object either by
        widget reference or by pane name, which acts as a unique id
        for a window pane. The returned `PaneInfo` object may then be
        modified to change a pane's look, state or position. After one
        or more modifications to the `PaneInfo`, `AuiManager.Update`
        should be called to realize the changes to the user interface.

        If the lookup failed (meaning the pane could not be found in
        the manager) GetPane returns an empty `PaneInfo`, a condition
        which can be checked by calling `PaneInfo.IsOk`.
        """
        if isinstance(item, wx.Window):
            return self._GetPaneByWidget(item)
        else:
            return self._GetPaneByName(item)

    def AddPane(self, window, info=None, caption=None):
        """
        AddPane(self, window, info=None, caption=None) -> bool

        AddPane tells the frame manager to start managing a child
        window. There are two versions of this function. The first
        verison accepts a `PaneInfo` object for the ``info`` parameter
        and allows the full spectrum of pane parameter
        possibilities. (Say that 3 times fast!)

        The second version is used for simpler user interfaces which
        do not require as much configuration.  In this case the
        ``info`` parameter specifies the direction property of the
        pane info, and defaults to ``wx.LEFT``.  The pane caption may
        also be specified as an extra parameter in this form.
        """
        if type(info) == AuiPaneInfo:
            return self._AddPane1(window, info)
        else:
            
            if info is None:
                info = wx.LEFT
            if caption is None:
                caption = ""
            return self._AddPane2(window, info, caption)

    SetFrame = wx._deprecated(SetManagedWindow,
                              "SetFrame is deprecated, use `SetManagedWindow` instead.")
    GetFrame = wx._deprecated(GetManagedWindow,
                              "GetFrame is deprecated, use `GetManagedWindow` instead.")

    AllPanes = property(GetAllPanes,doc="See `GetAllPanes`") 
    ArtProvider = property(GetArtProvider,SetArtProvider,doc="See `GetArtProvider` and `SetArtProvider`") 
    Flags = property(GetFlags,SetFlags,doc="See `GetFlags` and `SetFlags`") 
    ManagedWindow = property(GetManagedWindow,SetManagedWindow,doc="See `GetManagedWindow` and `SetManagedWindow`") 
_aui.AuiManager_swigregister(AuiManager)

def AuiManager_GetManager(*args, **kwargs):
  """AuiManager_GetManager(Window window) -> AuiManager"""
  return _aui.AuiManager_GetManager(*args, **kwargs)

class AuiManagerEvent(_core.Event):
    """Proxy of C++ AuiManagerEvent class"""
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    def __init__(self, *args, **kwargs): 
        """__init__(self, EventType type=wxEVT_NULL) -> AuiManagerEvent"""
        _aui.AuiManagerEvent_swiginit(self,_aui.new_AuiManagerEvent(*args, **kwargs))
    def Clone(*args, **kwargs):
        """Clone(self) -> Event"""
        return _aui.AuiManagerEvent_Clone(*args, **kwargs)

    def SetManager(*args, **kwargs):
        """SetManager(self, AuiManager mgr)"""
        return _aui.AuiManagerEvent_SetManager(*args, **kwargs)

    def SetPane(*args, **kwargs):
        """SetPane(self, AuiPaneInfo p)"""
        return _aui.AuiManagerEvent_SetPane(*args, **kwargs)

    def SetButton(*args, **kwargs):
        """SetButton(self, int b)"""
        return _aui.AuiManagerEvent_SetButton(*args, **kwargs)

    def SetDC(*args, **kwargs):
        """SetDC(self, DC pdc)"""
        return _aui.AuiManagerEvent_SetDC(*args, **kwargs)

    def GetManager(*args, **kwargs):
        """GetManager(self) -> AuiManager"""
        return _aui.AuiManagerEvent_GetManager(*args, **kwargs)

    def GetPane(*args, **kwargs):
        """GetPane(self) -> AuiPaneInfo"""
        return _aui.AuiManagerEvent_GetPane(*args, **kwargs)

    def GetButton(*args, **kwargs):
        """GetButton(self) -> int"""
        return _aui.AuiManagerEvent_GetButton(*args, **kwargs)

    def GetDC(*args, **kwargs):
        """GetDC(self) -> DC"""
        return _aui.AuiManagerEvent_GetDC(*args, **kwargs)

    def Veto(*args, **kwargs):
        """Veto(self, bool veto=True)"""
        return _aui.AuiManagerEvent_Veto(*args, **kwargs)

    def GetVeto(*args, **kwargs):
        """GetVeto(self) -> bool"""
        return _aui.AuiManagerEvent_GetVeto(*args, **kwargs)

    def SetCanVeto(*args, **kwargs):
        """SetCanVeto(self, bool can_veto)"""
        return _aui.AuiManagerEvent_SetCanVeto(*args, **kwargs)

    def CanVeto(*args, **kwargs):
        """CanVeto(self) -> bool"""
        return _aui.AuiManagerEvent_CanVeto(*args, **kwargs)

    manager = property(_aui.AuiManagerEvent_manager_get, _aui.AuiManagerEvent_manager_set)
    pane = property(_aui.AuiManagerEvent_pane_get, _aui.AuiManagerEvent_pane_set)
    button = property(_aui.AuiManagerEvent_button_get, _aui.AuiManagerEvent_button_set)
    veto_flag = property(_aui.AuiManagerEvent_veto_flag_get, _aui.AuiManagerEvent_veto_flag_set)
    canveto_flag = property(_aui.AuiManagerEvent_canveto_flag_get, _aui.AuiManagerEvent_canveto_flag_set)
    dc = property(_aui.AuiManagerEvent_dc_get, _aui.AuiManagerEvent_dc_set)
    Button = property(GetButton,SetButton,doc="See `GetButton` and `SetButton`") 
    DC = property(GetDC,SetDC,doc="See `GetDC` and `SetDC`") 
    Pane = property(GetPane,SetPane,doc="See `GetPane` and `SetPane`") 
_aui.AuiManagerEvent_swigregister(AuiManagerEvent)

class AuiDockInfo(object):
    """Proxy of C++ AuiDockInfo class"""
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    def __init__(self, *args, **kwargs): 
        """__init__(self) -> AuiDockInfo"""
        _aui.AuiDockInfo_swiginit(self,_aui.new_AuiDockInfo(*args, **kwargs))
    def IsOk(*args, **kwargs):
        """IsOk(self) -> bool"""
        return _aui.AuiDockInfo_IsOk(*args, **kwargs)

    def IsHorizontal(*args, **kwargs):
        """IsHorizontal(self) -> bool"""
        return _aui.AuiDockInfo_IsHorizontal(*args, **kwargs)

    def IsVertical(*args, **kwargs):
        """IsVertical(self) -> bool"""
        return _aui.AuiDockInfo_IsVertical(*args, **kwargs)

    panes = property(_aui.AuiDockInfo_panes_get, _aui.AuiDockInfo_panes_set)
    rect = property(_aui.AuiDockInfo_rect_get, _aui.AuiDockInfo_rect_set)
    dock_direction = property(_aui.AuiDockInfo_dock_direction_get, _aui.AuiDockInfo_dock_direction_set)
    dock_layer = property(_aui.AuiDockInfo_dock_layer_get, _aui.AuiDockInfo_dock_layer_set)
    dock_row = property(_aui.AuiDockInfo_dock_row_get, _aui.AuiDockInfo_dock_row_set)
    size = property(_aui.AuiDockInfo_size_get, _aui.AuiDockInfo_size_set)
    min_size = property(_aui.AuiDockInfo_min_size_get, _aui.AuiDockInfo_min_size_set)
    resizable = property(_aui.AuiDockInfo_resizable_get, _aui.AuiDockInfo_resizable_set)
    toolbar = property(_aui.AuiDockInfo_toolbar_get, _aui.AuiDockInfo_toolbar_set)
    fixed = property(_aui.AuiDockInfo_fixed_get, _aui.AuiDockInfo_fixed_set)
    reserved1 = property(_aui.AuiDockInfo_reserved1_get, _aui.AuiDockInfo_reserved1_set)
    __swig_destroy__ = _aui.delete_AuiDockInfo
    __del__ = lambda self : None;
_aui.AuiDockInfo_swigregister(AuiDockInfo)

class AuiDockUIPart(object):
    """Proxy of C++ AuiDockUIPart class"""
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    def __init__(self): raise AttributeError, "No constructor defined"
    __repr__ = _swig_repr
    typeCaption = _aui.AuiDockUIPart_typeCaption
    typeGripper = _aui.AuiDockUIPart_typeGripper
    typeDock = _aui.AuiDockUIPart_typeDock
    typeDockSizer = _aui.AuiDockUIPart_typeDockSizer
    typePane = _aui.AuiDockUIPart_typePane
    typePaneSizer = _aui.AuiDockUIPart_typePaneSizer
    typeBackground = _aui.AuiDockUIPart_typeBackground
    typePaneBorder = _aui.AuiDockUIPart_typePaneBorder
    typePaneButton = _aui.AuiDockUIPart_typePaneButton
    type = property(_aui.AuiDockUIPart_type_get, _aui.AuiDockUIPart_type_set)
    orientation = property(_aui.AuiDockUIPart_orientation_get, _aui.AuiDockUIPart_orientation_set)
    dock = property(_aui.AuiDockUIPart_dock_get, _aui.AuiDockUIPart_dock_set)
    pane = property(_aui.AuiDockUIPart_pane_get, _aui.AuiDockUIPart_pane_set)
    button = property(_aui.AuiDockUIPart_button_get, _aui.AuiDockUIPart_button_set)
    cont_sizer = property(_aui.AuiDockUIPart_cont_sizer_get, _aui.AuiDockUIPart_cont_sizer_set)
    sizer_item = property(_aui.AuiDockUIPart_sizer_item_get, _aui.AuiDockUIPart_sizer_item_set)
    rect = property(_aui.AuiDockUIPart_rect_get, _aui.AuiDockUIPart_rect_set)
    __swig_destroy__ = _aui.delete_AuiDockUIPart
    __del__ = lambda self : None;
_aui.AuiDockUIPart_swigregister(AuiDockUIPart)

class AuiPaneButton(object):
    """Proxy of C++ AuiPaneButton class"""
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    def __init__(self): raise AttributeError, "No constructor defined"
    __repr__ = _swig_repr
    button_id = property(_aui.AuiPaneButton_button_id_get, _aui.AuiPaneButton_button_id_set)
    __swig_destroy__ = _aui.delete_AuiPaneButton
    __del__ = lambda self : None;
_aui.AuiPaneButton_swigregister(AuiPaneButton)

wxEVT_AUI_PANE_BUTTON = _aui.wxEVT_AUI_PANE_BUTTON
wxEVT_AUI_PANE_CLOSE = _aui.wxEVT_AUI_PANE_CLOSE
wxEVT_AUI_PANE_MAXIMIZE = _aui.wxEVT_AUI_PANE_MAXIMIZE
wxEVT_AUI_PANE_RESTORE = _aui.wxEVT_AUI_PANE_RESTORE
wxEVT_AUI_RENDER = _aui.wxEVT_AUI_RENDER
wxEVT_AUI_FIND_MANAGER = _aui.wxEVT_AUI_FIND_MANAGER
EVT_AUI_PANE_BUTTON = wx.PyEventBinder( wxEVT_AUI_PANE_BUTTON )
EVT_AUI_PANE_CLOSE = wx.PyEventBinder( wxEVT_AUI_PANE_CLOSE )
EVT_AUI_PANE_MAXIMIZE = wx.PyEventBinder( wxEVT_AUI_PANE_MAXIMIZE )
EVT_AUI_PANE_RESTORE = wx.PyEventBinder( wxEVT_AUI_PANE_RESTORE )
EVT_AUI_RENDER = wx.PyEventBinder( wxEVT_AUI_RENDER )
EVT_AUI_FIND_MANAGER = wx.PyEventBinder( wxEVT_AUI_FIND_MANAGER )

class AuiDockArt(object):
    """Proxy of C++ AuiDockArt class"""
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    def __init__(self): raise AttributeError, "No constructor defined"
    __repr__ = _swig_repr
    __swig_destroy__ = _aui.delete_AuiDockArt
    __del__ = lambda self : None;
    def GetMetric(*args, **kwargs):
        """GetMetric(self, int id) -> int"""
        return _aui.AuiDockArt_GetMetric(*args, **kwargs)

    def SetMetric(*args, **kwargs):
        """SetMetric(self, int id, int new_val)"""
        return _aui.AuiDockArt_SetMetric(*args, **kwargs)

    def SetFont(*args, **kwargs):
        """SetFont(self, int id, Font font)"""
        return _aui.AuiDockArt_SetFont(*args, **kwargs)

    def GetFont(*args, **kwargs):
        """GetFont(self, int id) -> Font"""
        return _aui.AuiDockArt_GetFont(*args, **kwargs)

    def GetColour(*args, **kwargs):
        """GetColour(self, int id) -> Colour"""
        return _aui.AuiDockArt_GetColour(*args, **kwargs)

    def SetColour(*args, **kwargs):
        """SetColour(self, int id, Colour colour)"""
        return _aui.AuiDockArt_SetColour(*args, **kwargs)

    def GetColor(*args, **kwargs):
        """GetColor(self, int id) -> Colour"""
        return _aui.AuiDockArt_GetColor(*args, **kwargs)

    def SetColor(*args, **kwargs):
        """SetColor(self, int id, Colour color)"""
        return _aui.AuiDockArt_SetColor(*args, **kwargs)

    def DrawSash(*args, **kwargs):
        """DrawSash(self, DC dc, Window window, int orientation, Rect rect)"""
        return _aui.AuiDockArt_DrawSash(*args, **kwargs)

    def DrawBackground(*args, **kwargs):
        """DrawBackground(self, DC dc, Window window, int orientation, Rect rect)"""
        return _aui.AuiDockArt_DrawBackground(*args, **kwargs)

    def DrawCaption(*args, **kwargs):
        """DrawCaption(self, DC dc, Window window, String text, Rect rect, AuiPaneInfo pane)"""
        return _aui.AuiDockArt_DrawCaption(*args, **kwargs)

    def DrawGripper(*args, **kwargs):
        """DrawGripper(self, DC dc, Window window, Rect rect, AuiPaneInfo pane)"""
        return _aui.AuiDockArt_DrawGripper(*args, **kwargs)

    def DrawBorder(*args, **kwargs):
        """DrawBorder(self, DC dc, Window window, Rect rect, AuiPaneInfo pane)"""
        return _aui.AuiDockArt_DrawBorder(*args, **kwargs)

    def DrawPaneButton(*args, **kwargs):
        """
        DrawPaneButton(self, DC dc, Window window, int button, int button_state, 
            Rect rect, AuiPaneInfo pane)
        """
        return _aui.AuiDockArt_DrawPaneButton(*args, **kwargs)

_aui.AuiDockArt_swigregister(AuiDockArt)

class AuiDefaultDockArt(AuiDockArt):
    """Proxy of C++ AuiDefaultDockArt class"""
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    def __init__(self, *args, **kwargs): 
        """__init__(self) -> AuiDefaultDockArt"""
        _aui.AuiDefaultDockArt_swiginit(self,_aui.new_AuiDefaultDockArt(*args, **kwargs))
_aui.AuiDefaultDockArt_swigregister(AuiDefaultDockArt)

class AuiFloatingFrame(_windows.MiniFrame):
    """Proxy of C++ AuiFloatingFrame class"""
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    def __init__(self, *args, **kwargs): 
        """
        __init__(self, Window parent, AuiManager owner_mgr, AuiPaneInfo pane, 
            int id=ID_ANY, long style=wxRESIZE_BORDER|wxSYSTEM_MENU|wxCAPTION|wxFRAME_NO_TASKBAR|wxFRAME_FLOAT_ON_PARENT|wxCLIP_CHILDREN) -> AuiFloatingFrame
        """
        _aui.AuiFloatingFrame_swiginit(self,_aui.new_AuiFloatingFrame(*args, **kwargs))
    __swig_destroy__ = _aui.delete_AuiFloatingFrame
    __del__ = lambda self : None;
    def SetPaneWindow(*args, **kwargs):
        """SetPaneWindow(self, AuiPaneInfo pane)"""
        return _aui.AuiFloatingFrame_SetPaneWindow(*args, **kwargs)

    def GetOwnerManager(*args, **kwargs):
        """GetOwnerManager(self) -> AuiManager"""
        return _aui.AuiFloatingFrame_GetOwnerManager(*args, **kwargs)

_aui.AuiFloatingFrame_swigregister(AuiFloatingFrame)

AUI_NB_TOP = _aui.AUI_NB_TOP
AUI_NB_LEFT = _aui.AUI_NB_LEFT
AUI_NB_RIGHT = _aui.AUI_NB_RIGHT
AUI_NB_BOTTOM = _aui.AUI_NB_BOTTOM
AUI_NB_TAB_SPLIT = _aui.AUI_NB_TAB_SPLIT
AUI_NB_TAB_MOVE = _aui.AUI_NB_TAB_MOVE
AUI_NB_TAB_EXTERNAL_MOVE = _aui.AUI_NB_TAB_EXTERNAL_MOVE
AUI_NB_TAB_FIXED_WIDTH = _aui.AUI_NB_TAB_FIXED_WIDTH
AUI_NB_SCROLL_BUTTONS = _aui.AUI_NB_SCROLL_BUTTONS
AUI_NB_WINDOWLIST_BUTTON = _aui.AUI_NB_WINDOWLIST_BUTTON
AUI_NB_CLOSE_BUTTON = _aui.AUI_NB_CLOSE_BUTTON
AUI_NB_CLOSE_ON_ACTIVE_TAB = _aui.AUI_NB_CLOSE_ON_ACTIVE_TAB
AUI_NB_CLOSE_ON_ALL_TABS = _aui.AUI_NB_CLOSE_ON_ALL_TABS
AUI_NB_MIDDLE_CLICK_CLOSE = _aui.AUI_NB_MIDDLE_CLICK_CLOSE
AUI_NB_DEFAULT_STYLE = _aui.AUI_NB_DEFAULT_STYLE
class AuiNotebookEvent(_core.NotifyEvent):
    """Proxy of C++ AuiNotebookEvent class"""
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    def __init__(self, *args, **kwargs): 
        """__init__(self, EventType command_type=wxEVT_NULL, int win_id=0) -> AuiNotebookEvent"""
        _aui.AuiNotebookEvent_swiginit(self,_aui.new_AuiNotebookEvent(*args, **kwargs))
    def SetSelection(*args, **kwargs):
        """SetSelection(self, int s)"""
        return _aui.AuiNotebookEvent_SetSelection(*args, **kwargs)

    def GetSelection(*args, **kwargs):
        """
        GetSelection(self) -> int

        Returns item index for a listbox or choice selection event (not valid
        for a deselection).
        """
        return _aui.AuiNotebookEvent_GetSelection(*args, **kwargs)

    def SetOldSelection(*args, **kwargs):
        """SetOldSelection(self, int s)"""
        return _aui.AuiNotebookEvent_SetOldSelection(*args, **kwargs)

    def GetOldSelection(*args, **kwargs):
        """GetOldSelection(self) -> int"""
        return _aui.AuiNotebookEvent_GetOldSelection(*args, **kwargs)

    def SetDragSource(*args, **kwargs):
        """SetDragSource(self, AuiNotebook s)"""
        return _aui.AuiNotebookEvent_SetDragSource(*args, **kwargs)

    def GetDragSource(*args, **kwargs):
        """GetDragSource(self) -> AuiNotebook"""
        return _aui.AuiNotebookEvent_GetDragSource(*args, **kwargs)

    old_selection = property(_aui.AuiNotebookEvent_old_selection_get, _aui.AuiNotebookEvent_old_selection_set)
    selection = property(_aui.AuiNotebookEvent_selection_get, _aui.AuiNotebookEvent_selection_set)
    drag_source = property(_aui.AuiNotebookEvent_drag_source_get, _aui.AuiNotebookEvent_drag_source_set)
    OldSelection = property(GetOldSelection,SetOldSelection,doc="See `GetOldSelection` and `SetOldSelection`") 
    Selection = property(GetSelection,SetSelection,doc="See `GetSelection` and `SetSelection`") 
_aui.AuiNotebookEvent_swigregister(AuiNotebookEvent)

class AuiNotebookPage(object):
    """Proxy of C++ AuiNotebookPage class"""
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    def __init__(self): raise AttributeError, "No constructor defined"
    __repr__ = _swig_repr
    window = property(_aui.AuiNotebookPage_window_get, _aui.AuiNotebookPage_window_set)
    caption = property(_aui.AuiNotebookPage_caption_get, _aui.AuiNotebookPage_caption_set)
    bitmap = property(_aui.AuiNotebookPage_bitmap_get, _aui.AuiNotebookPage_bitmap_set)
    rect = property(_aui.AuiNotebookPage_rect_get, _aui.AuiNotebookPage_rect_set)
    active = property(_aui.AuiNotebookPage_active_get, _aui.AuiNotebookPage_active_set)
_aui.AuiNotebookPage_swigregister(AuiNotebookPage)

class AuiTabContainerButton(object):
    """Proxy of C++ AuiTabContainerButton class"""
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    def __init__(self): raise AttributeError, "No constructor defined"
    __repr__ = _swig_repr
    id = property(_aui.AuiTabContainerButton_id_get, _aui.AuiTabContainerButton_id_set)
    cur_state = property(_aui.AuiTabContainerButton_cur_state_get, _aui.AuiTabContainerButton_cur_state_set)
    location = property(_aui.AuiTabContainerButton_location_get, _aui.AuiTabContainerButton_location_set)
    bitmap = property(_aui.AuiTabContainerButton_bitmap_get, _aui.AuiTabContainerButton_bitmap_set)
    dis_bitmap = property(_aui.AuiTabContainerButton_dis_bitmap_get, _aui.AuiTabContainerButton_dis_bitmap_set)
    rect = property(_aui.AuiTabContainerButton_rect_get, _aui.AuiTabContainerButton_rect_set)
_aui.AuiTabContainerButton_swigregister(AuiTabContainerButton)

class AuiTabArt(object):
    """Proxy of C++ AuiTabArt class"""
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    def __init__(self): raise AttributeError, "No constructor defined"
    __repr__ = _swig_repr
    __swig_destroy__ = _aui.delete_AuiTabArt
    __del__ = lambda self : None;
    def Clone(*args, **kwargs):
        """Clone(self) -> AuiTabArt"""
        return _aui.AuiTabArt_Clone(*args, **kwargs)

    def SetFlags(*args, **kwargs):
        """SetFlags(self, int flags)"""
        return _aui.AuiTabArt_SetFlags(*args, **kwargs)

    def SetSizingInfo(*args, **kwargs):
        """SetSizingInfo(self, Size tab_ctrl_size, size_t tab_count)"""
        return _aui.AuiTabArt_SetSizingInfo(*args, **kwargs)

    def SetNormalFont(*args, **kwargs):
        """SetNormalFont(self, Font font)"""
        return _aui.AuiTabArt_SetNormalFont(*args, **kwargs)

    def SetSelectedFont(*args, **kwargs):
        """SetSelectedFont(self, Font font)"""
        return _aui.AuiTabArt_SetSelectedFont(*args, **kwargs)

    def SetMeasuringFont(*args, **kwargs):
        """SetMeasuringFont(self, Font font)"""
        return _aui.AuiTabArt_SetMeasuringFont(*args, **kwargs)

    def DrawBackground(*args, **kwargs):
        """DrawBackground(self, DC dc, Window wnd, Rect rect)"""
        return _aui.AuiTabArt_DrawBackground(*args, **kwargs)

    def ShowDropDown(*args, **kwargs):
        """ShowDropDown(self, Window wnd, wxAuiNotebookPageArray items, int active_idx) -> int"""
        return _aui.AuiTabArt_ShowDropDown(*args, **kwargs)

    def GetIndentSize(*args, **kwargs):
        """GetIndentSize(self) -> int"""
        return _aui.AuiTabArt_GetIndentSize(*args, **kwargs)

    def GetBestTabCtrlSize(*args, **kwargs):
        """GetBestTabCtrlSize(self, Window wnd, wxAuiNotebookPageArray pages, Size required_bmp_size) -> int"""
        return _aui.AuiTabArt_GetBestTabCtrlSize(*args, **kwargs)

    def DrawTab(*args, **kwargs):
        """
        DrawTab(self, DC dc, Window wnd, AuiNotebookPage pane, Rect in_rect, 
            int close_button_state) -> PyObject
        """
        return _aui.AuiTabArt_DrawTab(*args, **kwargs)

    def DrawButton(*args, **kwargs):
        """
        DrawButton(self, DC dc, Window wnd, Rect in_rect, int bitmap_id, int button_state, 
            int orientation) -> PyObject
        """
        return _aui.AuiTabArt_DrawButton(*args, **kwargs)

    def GetTabSize(*args, **kwargs):
        """
        GetTabSize(self, DC dc, Window wnd, String caption, Bitmap bitmap, bool active, 
            int close_button_state) -> PyObject
        """
        return _aui.AuiTabArt_GetTabSize(*args, **kwargs)

_aui.AuiTabArt_swigregister(AuiTabArt)

class AuiDefaultTabArt(AuiTabArt):
    """Proxy of C++ AuiDefaultTabArt class"""
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    def __init__(self, *args, **kwargs): 
        """__init__(self) -> AuiDefaultTabArt"""
        _aui.AuiDefaultTabArt_swiginit(self,_aui.new_AuiDefaultTabArt(*args, **kwargs))
    __swig_destroy__ = _aui.delete_AuiDefaultTabArt
    __del__ = lambda self : None;
_aui.AuiDefaultTabArt_swigregister(AuiDefaultTabArt)

class AuiSimpleTabArt(AuiTabArt):
    """Proxy of C++ AuiSimpleTabArt class"""
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    def __init__(self, *args, **kwargs): 
        """__init__(self) -> AuiSimpleTabArt"""
        _aui.AuiSimpleTabArt_swiginit(self,_aui.new_AuiSimpleTabArt(*args, **kwargs))
    __swig_destroy__ = _aui.delete_AuiSimpleTabArt
    __del__ = lambda self : None;
_aui.AuiSimpleTabArt_swigregister(AuiSimpleTabArt)

class AuiTabContainer(object):
    """Proxy of C++ AuiTabContainer class"""
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    def __init__(self, *args, **kwargs): 
        """__init__(self) -> AuiTabContainer"""
        _aui.AuiTabContainer_swiginit(self,_aui.new_AuiTabContainer(*args, **kwargs))
    __swig_destroy__ = _aui.delete_AuiTabContainer
    __del__ = lambda self : None;
    def SetArtProvider(*args, **kwargs):
        """SetArtProvider(self, AuiTabArt art)"""
        return _aui.AuiTabContainer_SetArtProvider(*args, **kwargs)

    def GetArtProvider(*args, **kwargs):
        """GetArtProvider(self) -> AuiTabArt"""
        return _aui.AuiTabContainer_GetArtProvider(*args, **kwargs)

    def SetFlags(*args, **kwargs):
        """SetFlags(self, int flags)"""
        return _aui.AuiTabContainer_SetFlags(*args, **kwargs)

    def GetFlags(*args, **kwargs):
        """GetFlags(self) -> int"""
        return _aui.AuiTabContainer_GetFlags(*args, **kwargs)

    def AddPage(*args, **kwargs):
        """AddPage(self, Window page, AuiNotebookPage info) -> bool"""
        return _aui.AuiTabContainer_AddPage(*args, **kwargs)

    def InsertPage(*args, **kwargs):
        """InsertPage(self, Window page, AuiNotebookPage info, size_t idx) -> bool"""
        return _aui.AuiTabContainer_InsertPage(*args, **kwargs)

    def MovePage(*args, **kwargs):
        """MovePage(self, Window page, size_t new_idx) -> bool"""
        return _aui.AuiTabContainer_MovePage(*args, **kwargs)

    def RemovePage(*args, **kwargs):
        """RemovePage(self, Window page) -> bool"""
        return _aui.AuiTabContainer_RemovePage(*args, **kwargs)

    def SetActivePage(*args):
        """
        SetActivePage(self, Window page) -> bool
        SetActivePage(self, size_t page) -> bool
        """
        return _aui.AuiTabContainer_SetActivePage(*args)

    def SetNoneActive(*args, **kwargs):
        """SetNoneActive(self)"""
        return _aui.AuiTabContainer_SetNoneActive(*args, **kwargs)

    def GetActivePage(*args, **kwargs):
        """GetActivePage(self) -> int"""
        return _aui.AuiTabContainer_GetActivePage(*args, **kwargs)

    def TabHitTest(*args, **kwargs):
        """TabHitTest(self, int x, int y, Window hit) -> bool"""
        return _aui.AuiTabContainer_TabHitTest(*args, **kwargs)

    def ButtonHitTest(*args, **kwargs):
        """ButtonHitTest(self, int x, int y, AuiTabContainerButton hit) -> bool"""
        return _aui.AuiTabContainer_ButtonHitTest(*args, **kwargs)

    def GetWindowFromIdx(*args, **kwargs):
        """GetWindowFromIdx(self, size_t idx) -> Window"""
        return _aui.AuiTabContainer_GetWindowFromIdx(*args, **kwargs)

    def GetIdxFromWindow(*args, **kwargs):
        """GetIdxFromWindow(self, Window page) -> int"""
        return _aui.AuiTabContainer_GetIdxFromWindow(*args, **kwargs)

    def GetPageCount(*args, **kwargs):
        """GetPageCount(self) -> size_t"""
        return _aui.AuiTabContainer_GetPageCount(*args, **kwargs)

    def GetPage(*args, **kwargs):
        """GetPage(self, size_t idx) -> AuiNotebookPage"""
        return _aui.AuiTabContainer_GetPage(*args, **kwargs)

    def GetPages(*args, **kwargs):
        """GetPages(self) -> wxAuiNotebookPageArray"""
        return _aui.AuiTabContainer_GetPages(*args, **kwargs)

    def SetNormalFont(*args, **kwargs):
        """SetNormalFont(self, Font normal_font)"""
        return _aui.AuiTabContainer_SetNormalFont(*args, **kwargs)

    def SetSelectedFont(*args, **kwargs):
        """SetSelectedFont(self, Font selected_font)"""
        return _aui.AuiTabContainer_SetSelectedFont(*args, **kwargs)

    def SetMeasuringFont(*args, **kwargs):
        """SetMeasuringFont(self, Font measuring_font)"""
        return _aui.AuiTabContainer_SetMeasuringFont(*args, **kwargs)

    def DoShowHide(*args, **kwargs):
        """DoShowHide(self)"""
        return _aui.AuiTabContainer_DoShowHide(*args, **kwargs)

    def SetRect(*args, **kwargs):
        """SetRect(self, Rect rect)"""
        return _aui.AuiTabContainer_SetRect(*args, **kwargs)

    def RemoveButton(*args, **kwargs):
        """RemoveButton(self, int id)"""
        return _aui.AuiTabContainer_RemoveButton(*args, **kwargs)

    def AddButton(*args, **kwargs):
        """
        AddButton(self, int id, int location, Bitmap normal_bitmap=wxNullBitmap, 
            Bitmap disabled_bitmap=wxNullBitmap)
        """
        return _aui.AuiTabContainer_AddButton(*args, **kwargs)

    def GetTabOffset(*args, **kwargs):
        """GetTabOffset(self) -> size_t"""
        return _aui.AuiTabContainer_GetTabOffset(*args, **kwargs)

    def SetTabOffset(*args, **kwargs):
        """SetTabOffset(self, size_t offset)"""
        return _aui.AuiTabContainer_SetTabOffset(*args, **kwargs)

    def IsTabVisible(*args, **kwargs):
        """IsTabVisible(self, int tabPage, int tabOffset, DC dc, Window wnd) -> bool"""
        return _aui.AuiTabContainer_IsTabVisible(*args, **kwargs)

    def MakeTabVisible(*args, **kwargs):
        """MakeTabVisible(self, int tabPage, Window win)"""
        return _aui.AuiTabContainer_MakeTabVisible(*args, **kwargs)

    ActivePage = property(GetActivePage,SetActivePage,doc="See `GetActivePage` and `SetActivePage`") 
    PageCount = property(GetPageCount,doc="See `GetPageCount`") 
    Pages = property(GetPages,doc="See `GetPages`") 
_aui.AuiTabContainer_swigregister(AuiTabContainer)

class AuiTabCtrl(_core.Control,AuiTabContainer):
    """Proxy of C++ AuiTabCtrl class"""
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    def __init__(self, *args, **kwargs): 
        """
        __init__(self, Window parent, int id=ID_ANY, Point pos=DefaultPosition, 
            Size size=DefaultSize, long style=0) -> AuiTabCtrl
        """
        _aui.AuiTabCtrl_swiginit(self,_aui.new_AuiTabCtrl(*args, **kwargs))
        self._setOORInfo(self)

    __swig_destroy__ = _aui.delete_AuiTabCtrl
    __del__ = lambda self : None;
    def IsDragging(*args, **kwargs):
        """IsDragging(self) -> bool"""
        return _aui.AuiTabCtrl_IsDragging(*args, **kwargs)

_aui.AuiTabCtrl_swigregister(AuiTabCtrl)

class AuiNotebook(_core.Control):
    """Proxy of C++ AuiNotebook class"""
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    def __init__(self, *args, **kwargs): 
        """
        __init__(self, Window parent, int id=ID_ANY, Point pos=DefaultPosition, 
            Size size=DefaultSize, long style=AUI_NB_DEFAULT_STYLE) -> AuiNotebook
        """
        _aui.AuiNotebook_swiginit(self,_aui.new_AuiNotebook(*args, **kwargs))
        self._setOORInfo(self)

    __swig_destroy__ = _aui.delete_AuiNotebook
    __del__ = lambda self : None;
    def Create(*args, **kwargs):
        """
        Create(self, Window parent, int id=ID_ANY, Point pos=DefaultPosition, 
            Size size=DefaultSize, long style=0) -> bool

        Do the 2nd phase and create the GUI control.
        """
        return _aui.AuiNotebook_Create(*args, **kwargs)

    def SetArtProvider(*args, **kwargs):
        """SetArtProvider(self, AuiTabArt art)"""
        return _aui.AuiNotebook_SetArtProvider(*args, **kwargs)

    def GetArtProvider(*args, **kwargs):
        """GetArtProvider(self) -> AuiTabArt"""
        return _aui.AuiNotebook_GetArtProvider(*args, **kwargs)

    def SetUniformBitmapSize(*args, **kwargs):
        """SetUniformBitmapSize(self, Size size)"""
        return _aui.AuiNotebook_SetUniformBitmapSize(*args, **kwargs)

    def SetTabCtrlHeight(*args, **kwargs):
        """SetTabCtrlHeight(self, int height)"""
        return _aui.AuiNotebook_SetTabCtrlHeight(*args, **kwargs)

    def AddPage(*args, **kwargs):
        """AddPage(self, Window page, String caption, bool select=False, Bitmap bitmap=wxNullBitmap) -> bool"""
        return _aui.AuiNotebook_AddPage(*args, **kwargs)

    def InsertPage(*args, **kwargs):
        """
        InsertPage(self, size_t page_idx, Window page, String caption, bool select=False, 
            Bitmap bitmap=wxNullBitmap) -> bool
        """
        return _aui.AuiNotebook_InsertPage(*args, **kwargs)

    def DeletePage(*args, **kwargs):
        """DeletePage(self, size_t page) -> bool"""
        return _aui.AuiNotebook_DeletePage(*args, **kwargs)

    def RemovePage(*args, **kwargs):
        """RemovePage(self, size_t page) -> bool"""
        return _aui.AuiNotebook_RemovePage(*args, **kwargs)

    def GetPageCount(*args, **kwargs):
        """GetPageCount(self) -> size_t"""
        return _aui.AuiNotebook_GetPageCount(*args, **kwargs)

    def GetPage(*args, **kwargs):
        """GetPage(self, size_t page_idx) -> Window"""
        return _aui.AuiNotebook_GetPage(*args, **kwargs)

    def GetPageIndex(*args, **kwargs):
        """GetPageIndex(self, Window page_wnd) -> int"""
        return _aui.AuiNotebook_GetPageIndex(*args, **kwargs)

    def SetPageText(*args, **kwargs):
        """SetPageText(self, size_t page, String text) -> bool"""
        return _aui.AuiNotebook_SetPageText(*args, **kwargs)

    def GetPageText(*args, **kwargs):
        """GetPageText(self, size_t page_idx) -> String"""
        return _aui.AuiNotebook_GetPageText(*args, **kwargs)

    def SetPageBitmap(*args, **kwargs):
        """SetPageBitmap(self, size_t page, Bitmap bitmap) -> bool"""
        return _aui.AuiNotebook_SetPageBitmap(*args, **kwargs)

    def GetPageBitmap(*args, **kwargs):
        """GetPageBitmap(self, size_t page_idx) -> Bitmap"""
        return _aui.AuiNotebook_GetPageBitmap(*args, **kwargs)

    def SetSelection(*args, **kwargs):
        """SetSelection(self, size_t new_page) -> size_t"""
        return _aui.AuiNotebook_SetSelection(*args, **kwargs)

    def GetSelection(*args, **kwargs):
        """GetSelection(self) -> int"""
        return _aui.AuiNotebook_GetSelection(*args, **kwargs)

    def Split(*args, **kwargs):
        """Split(self, size_t page, int direction)"""
        return _aui.AuiNotebook_Split(*args, **kwargs)

    def GetAuiManager(*args, **kwargs):
        """GetAuiManager(self) -> AuiManager"""
        return _aui.AuiNotebook_GetAuiManager(*args, **kwargs)

    def SetNormalFont(*args, **kwargs):
        """SetNormalFont(self, Font font)"""
        return _aui.AuiNotebook_SetNormalFont(*args, **kwargs)

    def SetSelectedFont(*args, **kwargs):
        """SetSelectedFont(self, Font font)"""
        return _aui.AuiNotebook_SetSelectedFont(*args, **kwargs)

    def SetMeasuringFont(*args, **kwargs):
        """SetMeasuringFont(self, Font font)"""
        return _aui.AuiNotebook_SetMeasuringFont(*args, **kwargs)

    def GetTabCtrlHeight(*args, **kwargs):
        """GetTabCtrlHeight(self) -> int"""
        return _aui.AuiNotebook_GetTabCtrlHeight(*args, **kwargs)

    def GetHeightForPageHeight(*args, **kwargs):
        """GetHeightForPageHeight(self, int pageHeight) -> int"""
        return _aui.AuiNotebook_GetHeightForPageHeight(*args, **kwargs)

    def AdvanceSelection(*args, **kwargs):
        """AdvanceSelection(self, bool forward=True)"""
        return _aui.AuiNotebook_AdvanceSelection(*args, **kwargs)

    def ShowWindowMenu(*args, **kwargs):
        """ShowWindowMenu(self) -> bool"""
        return _aui.AuiNotebook_ShowWindowMenu(*args, **kwargs)

    PageCount = property(GetPageCount,doc="See `GetPageCount`") 
    Selection = property(GetSelection,SetSelection,doc="See `GetSelection` and `SetSelection`") 
_aui.AuiNotebook_swigregister(AuiNotebook)

def PreAuiNotebook(*args, **kwargs):
    """PreAuiNotebook() -> AuiNotebook"""
    val = _aui.new_PreAuiNotebook(*args, **kwargs)
    val._setOORInfo(val)
    return val

wxEVT_COMMAND_AUINOTEBOOK_PAGE_CLOSE = _aui.wxEVT_COMMAND_AUINOTEBOOK_PAGE_CLOSE
wxEVT_COMMAND_AUINOTEBOOK_PAGE_CLOSED = _aui.wxEVT_COMMAND_AUINOTEBOOK_PAGE_CLOSED
wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED = _aui.wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED
wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGING = _aui.wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGING
wxEVT_COMMAND_AUINOTEBOOK_BUTTON = _aui.wxEVT_COMMAND_AUINOTEBOOK_BUTTON
wxEVT_COMMAND_AUINOTEBOOK_BEGIN_DRAG = _aui.wxEVT_COMMAND_AUINOTEBOOK_BEGIN_DRAG
wxEVT_COMMAND_AUINOTEBOOK_END_DRAG = _aui.wxEVT_COMMAND_AUINOTEBOOK_END_DRAG
wxEVT_COMMAND_AUINOTEBOOK_DRAG_MOTION = _aui.wxEVT_COMMAND_AUINOTEBOOK_DRAG_MOTION
wxEVT_COMMAND_AUINOTEBOOK_ALLOW_DND = _aui.wxEVT_COMMAND_AUINOTEBOOK_ALLOW_DND
wxEVT_COMMAND_AUINOTEBOOK_DRAG_DONE = _aui.wxEVT_COMMAND_AUINOTEBOOK_DRAG_DONE
wxEVT_COMMAND_AUINOTEBOOK_BG_DCLICK = _aui.wxEVT_COMMAND_AUINOTEBOOK_BG_DCLICK
wxEVT_COMMAND_AUINOTEBOOK_TAB_MIDDLE_DOWN = _aui.wxEVT_COMMAND_AUINOTEBOOK_TAB_MIDDLE_DOWN
wxEVT_COMMAND_AUINOTEBOOK_TAB_MIDDLE_UP = _aui.wxEVT_COMMAND_AUINOTEBOOK_TAB_MIDDLE_UP
wxEVT_COMMAND_AUINOTEBOOK_TAB_RIGHT_DOWN = _aui.wxEVT_COMMAND_AUINOTEBOOK_TAB_RIGHT_DOWN
wxEVT_COMMAND_AUINOTEBOOK_TAB_RIGHT_UP = _aui.wxEVT_COMMAND_AUINOTEBOOK_TAB_RIGHT_UP
EVT_AUINOTEBOOK_PAGE_CLOSE = wx.PyEventBinder( wxEVT_COMMAND_AUINOTEBOOK_PAGE_CLOSE, 1 )
EVT_AUINOTEBOOK_PAGE_CLOSED = wx.PyEventBinder( wxEVT_COMMAND_AUINOTEBOOK_PAGE_CLOSED, 1 )
EVT_AUINOTEBOOK_PAGE_CHANGED = wx.PyEventBinder( wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED, 1 )
EVT_AUINOTEBOOK_PAGE_CHANGING = wx.PyEventBinder( wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGING, 1 )
EVT_AUINOTEBOOK_BUTTON = wx.PyEventBinder( wxEVT_COMMAND_AUINOTEBOOK_BUTTON, 1 )
EVT_AUINOTEBOOK_BEGIN_DRAG = wx.PyEventBinder( wxEVT_COMMAND_AUINOTEBOOK_BEGIN_DRAG, 1 )
EVT_AUINOTEBOOK_END_DRAG = wx.PyEventBinder( wxEVT_COMMAND_AUINOTEBOOK_END_DRAG, 1 )
EVT_AUINOTEBOOK_DRAG_MOTION = wx.PyEventBinder( wxEVT_COMMAND_AUINOTEBOOK_DRAG_MOTION, 1 )
EVT_AUINOTEBOOK_ALLOW_DND = wx.PyEventBinder( wxEVT_COMMAND_AUINOTEBOOK_ALLOW_DND, 1 )
EVT_AUINOTEBOOK_DRAG_DONE = wx.PyEventBinder( wxEVT_COMMAND_AUINOTEBOOK_DRAG_DONE, 1 )
EVT_AUINOTEBOOK_BG_DCLICK = wx.PyEventBinder( wxEVT_COMMAND_AUINOTEBOOK_BG_DCLICK, 1 )
EVT_AUINOTEBOOK_TAB_MIDDLE_DOWN = wx.PyEventBinder( wxEVT_COMMAND_AUINOTEBOOK_TAB_MIDDLE_DOWN, 1 )
EVT_AUINOTEBOOK_TAB_MIDDLE_UP  = wx.PyEventBinder( wxEVT_COMMAND_AUINOTEBOOK_TAB_MIDDLE_UP , 1 )
EVT_AUINOTEBOOK_TAB_RIGHT_DOWN = wx.PyEventBinder( wxEVT_COMMAND_AUINOTEBOOK_TAB_RIGHT_DOWN, 1 )
EVT_AUINOTEBOOK_TAB_RIGHT_UP = wx.PyEventBinder( wxEVT_COMMAND_AUINOTEBOOK_TAB_RIGHT_UP, 1 )

class AuiMDIParentFrame(_windows.Frame):
    """Proxy of C++ AuiMDIParentFrame class"""
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    def __init__(self, *args, **kwargs): 
        """
        __init__(self, Window parent, int winid, String title, Point pos=DefaultPosition, 
            Size size=DefaultSize, long style=wxDEFAULT_FRAME_STYLE|wxVSCROLL|wxHSCROLL, 
            String name=wxFrameNameStr) -> AuiMDIParentFrame
        """
        _aui.AuiMDIParentFrame_swiginit(self,_aui.new_AuiMDIParentFrame(*args, **kwargs))
        self._setOORInfo(self)

    def Create(*args, **kwargs):
        """
        Create(self, Window parent, int winid, String title, Point pos=DefaultPosition, 
            Size size=DefaultSize, long style=wxDEFAULT_FRAME_STYLE|wxVSCROLL|wxHSCROLL, 
            String name=wxFrameNameStr) -> bool
        """
        return _aui.AuiMDIParentFrame_Create(*args, **kwargs)

    def SetArtProvider(*args, **kwargs):
        """SetArtProvider(self, AuiTabArt provider)"""
        return _aui.AuiMDIParentFrame_SetArtProvider(*args, **kwargs)

    def GetArtProvider(*args, **kwargs):
        """GetArtProvider(self) -> AuiTabArt"""
        return _aui.AuiMDIParentFrame_GetArtProvider(*args, **kwargs)

    def GetWindowMenu(*args, **kwargs):
        """GetWindowMenu(self) -> Menu"""
        return _aui.AuiMDIParentFrame_GetWindowMenu(*args, **kwargs)

    def SetWindowMenu(*args, **kwargs):
        """SetWindowMenu(self, Menu pMenu)"""
        return _aui.AuiMDIParentFrame_SetWindowMenu(*args, **kwargs)

    def SetChildMenuBar(*args, **kwargs):
        """SetChildMenuBar(self, AuiMDIChildFrame pChild)"""
        return _aui.AuiMDIParentFrame_SetChildMenuBar(*args, **kwargs)

    def ProcessEvent(*args, **kwargs):
        """ProcessEvent(self, Event event) -> bool"""
        return _aui.AuiMDIParentFrame_ProcessEvent(*args, **kwargs)

    def SetActiveChild(*args, **kwargs):
        """SetActiveChild(self, AuiMDIChildFrame pChildFrame)"""
        return _aui.AuiMDIParentFrame_SetActiveChild(*args, **kwargs)

    def OnCreateClient(*args, **kwargs):
        """OnCreateClient(self) -> AuiMDIClientWindow"""
        return _aui.AuiMDIParentFrame_OnCreateClient(*args, **kwargs)

    def Cascade(*args, **kwargs):
        """Cascade(self)"""
        return _aui.AuiMDIParentFrame_Cascade(*args, **kwargs)

    def Tile(*args, **kwargs):
        """Tile(self, int orient=HORIZONTAL)"""
        return _aui.AuiMDIParentFrame_Tile(*args, **kwargs)

    def ArrangeIcons(*args, **kwargs):
        """ArrangeIcons(self)"""
        return _aui.AuiMDIParentFrame_ArrangeIcons(*args, **kwargs)

    def ActivateNext(*args, **kwargs):
        """ActivateNext(self)"""
        return _aui.AuiMDIParentFrame_ActivateNext(*args, **kwargs)

    def ActivatePrevious(*args, **kwargs):
        """ActivatePrevious(self)"""
        return _aui.AuiMDIParentFrame_ActivatePrevious(*args, **kwargs)

    def GetNotebook(*args, **kwargs):
        """GetNotebook(self) -> AuiNotebook"""
        return _aui.AuiMDIParentFrame_GetNotebook(*args, **kwargs)

    def GetActiveChild(*args, **kwargs):
        """GetActiveChild(self) -> AuiMDIChildFrame"""
        return _aui.AuiMDIParentFrame_GetActiveChild(*args, **kwargs)

    def GetClientWindow(*args, **kwargs):
        """GetClientWindow(self) -> AuiMDIClientWindow"""
        return _aui.AuiMDIParentFrame_GetClientWindow(*args, **kwargs)

_aui.AuiMDIParentFrame_swigregister(AuiMDIParentFrame)

def PreAuiMDIParentFrame(*args, **kwargs):
    """PreAuiMDIParentFrame() -> AuiMDIParentFrame"""
    val = _aui.new_PreAuiMDIParentFrame(*args, **kwargs)
    val._setOORInfo(val)
    return val

class AuiMDIChildFrame(_windows.Panel):
    """Proxy of C++ AuiMDIChildFrame class"""
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    def __init__(self, *args, **kwargs): 
        """
        __init__(self, AuiMDIParentFrame parent, int winid, String title, 
            Point pos=DefaultPosition, Size size=DefaultSize, 
            long style=DEFAULT_FRAME_STYLE, String name=wxFrameNameStr) -> AuiMDIChildFrame
        """
        _aui.AuiMDIChildFrame_swiginit(self,_aui.new_AuiMDIChildFrame(*args, **kwargs))
        self._setOORInfo(self)

    def Create(*args, **kwargs):
        """
        Create(self, AuiMDIParentFrame parent, int winid, String title, 
            Point pos=DefaultPosition, Size size=DefaultSize, 
            long style=DEFAULT_FRAME_STYLE, String name=wxFrameNameStr) -> bool
        """
        return _aui.AuiMDIChildFrame_Create(*args, **kwargs)

    def SetMenuBar(*args, **kwargs):
        """SetMenuBar(self, MenuBar menu_bar)"""
        return _aui.AuiMDIChildFrame_SetMenuBar(*args, **kwargs)

    def GetMenuBar(*args, **kwargs):
        """GetMenuBar(self) -> MenuBar"""
        return _aui.AuiMDIChildFrame_GetMenuBar(*args, **kwargs)

    def SetTitle(*args, **kwargs):
        """SetTitle(self, String title)"""
        return _aui.AuiMDIChildFrame_SetTitle(*args, **kwargs)

    def GetTitle(*args, **kwargs):
        """GetTitle(self) -> String"""
        return _aui.AuiMDIChildFrame_GetTitle(*args, **kwargs)

    def SetIcons(*args, **kwargs):
        """SetIcons(self, wxIconBundle icons)"""
        return _aui.AuiMDIChildFrame_SetIcons(*args, **kwargs)

    def GetIcons(*args, **kwargs):
        """GetIcons(self) -> wxIconBundle"""
        return _aui.AuiMDIChildFrame_GetIcons(*args, **kwargs)

    def SetIcon(*args, **kwargs):
        """SetIcon(self, Icon icon)"""
        return _aui.AuiMDIChildFrame_SetIcon(*args, **kwargs)

    def GetIcon(*args, **kwargs):
        """GetIcon(self) -> Icon"""
        return _aui.AuiMDIChildFrame_GetIcon(*args, **kwargs)

    def Activate(*args, **kwargs):
        """Activate(self)"""
        return _aui.AuiMDIChildFrame_Activate(*args, **kwargs)

    def DoSetSizeHints(*args, **kwargs):
        """
        DoSetSizeHints(self, int minW, int minH, int maxW=wxDefaultCoord, int maxH=wxDefaultCoord, 
            int incW=wxDefaultCoord, int incH=wxDefaultCoord)
        """
        return _aui.AuiMDIChildFrame_DoSetSizeHints(*args, **kwargs)

    def Maximize(*args, **kwargs):
        """Maximize(self, bool maximize=True)"""
        return _aui.AuiMDIChildFrame_Maximize(*args, **kwargs)

    def Restore(*args, **kwargs):
        """Restore(self)"""
        return _aui.AuiMDIChildFrame_Restore(*args, **kwargs)

    def Iconize(*args, **kwargs):
        """Iconize(self, bool iconize=True)"""
        return _aui.AuiMDIChildFrame_Iconize(*args, **kwargs)

    def IsMaximized(*args, **kwargs):
        """IsMaximized(self) -> bool"""
        return _aui.AuiMDIChildFrame_IsMaximized(*args, **kwargs)

    def IsIconized(*args, **kwargs):
        """IsIconized(self) -> bool"""
        return _aui.AuiMDIChildFrame_IsIconized(*args, **kwargs)

    def ShowFullScreen(*args, **kwargs):
        """ShowFullScreen(self, bool show, long style) -> bool"""
        return _aui.AuiMDIChildFrame_ShowFullScreen(*args, **kwargs)

    def IsFullScreen(*args, **kwargs):
        """IsFullScreen(self) -> bool"""
        return _aui.AuiMDIChildFrame_IsFullScreen(*args, **kwargs)

    def OnMenuHighlight(*args, **kwargs):
        """OnMenuHighlight(self, MenuEvent evt)"""
        return _aui.AuiMDIChildFrame_OnMenuHighlight(*args, **kwargs)

    def OnActivate(*args, **kwargs):
        """OnActivate(self, ActivateEvent evt)"""
        return _aui.AuiMDIChildFrame_OnActivate(*args, **kwargs)

    def OnCloseWindow(*args, **kwargs):
        """OnCloseWindow(self, CloseEvent evt)"""
        return _aui.AuiMDIChildFrame_OnCloseWindow(*args, **kwargs)

    def SetMDIParentFrame(*args, **kwargs):
        """SetMDIParentFrame(self, AuiMDIParentFrame parent)"""
        return _aui.AuiMDIChildFrame_SetMDIParentFrame(*args, **kwargs)

    def GetMDIParentFrame(*args, **kwargs):
        """GetMDIParentFrame(self) -> AuiMDIParentFrame"""
        return _aui.AuiMDIChildFrame_GetMDIParentFrame(*args, **kwargs)

    def ApplyMDIChildFrameRect(*args, **kwargs):
        """ApplyMDIChildFrameRect(self)"""
        return _aui.AuiMDIChildFrame_ApplyMDIChildFrameRect(*args, **kwargs)

    def DoShow(*args, **kwargs):
        """DoShow(self, bool show)"""
        return _aui.AuiMDIChildFrame_DoShow(*args, **kwargs)

_aui.AuiMDIChildFrame_swigregister(AuiMDIChildFrame)

def PreAuiMDIChildFrame(*args, **kwargs):
    """PreAuiMDIChildFrame() -> AuiMDIChildFrame"""
    val = _aui.new_PreAuiMDIChildFrame(*args, **kwargs)
    val._setOORInfo(val)
    return val

class AuiMDIClientWindow(AuiNotebook):
    """Proxy of C++ AuiMDIClientWindow class"""
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    def __init__(self, *args, **kwargs): 
        """__init__(self, AuiMDIParentFrame parent, long style=0) -> AuiMDIClientWindow"""
        _aui.AuiMDIClientWindow_swiginit(self,_aui.new_AuiMDIClientWindow(*args, **kwargs))
        self._setOORInfo(self)

    def CreateClient(*args, **kwargs):
        """CreateClient(self, AuiMDIParentFrame parent, long style=wxVSCROLL|wxHSCROLL) -> bool"""
        return _aui.AuiMDIClientWindow_CreateClient(*args, **kwargs)

    def SetSelection(*args, **kwargs):
        """SetSelection(self, size_t page) -> int"""
        return _aui.AuiMDIClientWindow_SetSelection(*args, **kwargs)

_aui.AuiMDIClientWindow_swigregister(AuiMDIClientWindow)

def PreAuiMDIClientWindow(*args, **kwargs):
    """PreAuiMDIClientWindow() -> AuiMDIClientWindow"""
    val = _aui.new_PreAuiMDIClientWindow(*args, **kwargs)
    val._setOORInfo(val)
    return val

AUI_TB_TEXT = _aui.AUI_TB_TEXT
AUI_TB_NO_TOOLTIPS = _aui.AUI_TB_NO_TOOLTIPS
AUI_TB_NO_AUTORESIZE = _aui.AUI_TB_NO_AUTORESIZE
AUI_TB_GRIPPER = _aui.AUI_TB_GRIPPER
AUI_TB_OVERFLOW = _aui.AUI_TB_OVERFLOW
AUI_TB_VERTICAL = _aui.AUI_TB_VERTICAL
AUI_TB_HORZ_LAYOUT = _aui.AUI_TB_HORZ_LAYOUT
AUI_TB_HORZ_TEXT = _aui.AUI_TB_HORZ_TEXT
AUI_TB_DEFAULT_STYLE = _aui.AUI_TB_DEFAULT_STYLE
AUI_TBART_SEPARATOR_SIZE = _aui.AUI_TBART_SEPARATOR_SIZE
AUI_TBART_GRIPPER_SIZE = _aui.AUI_TBART_GRIPPER_SIZE
AUI_TBART_OVERFLOW_SIZE = _aui.AUI_TBART_OVERFLOW_SIZE
AUI_TBTOOL_TEXT_LEFT = _aui.AUI_TBTOOL_TEXT_LEFT
AUI_TBTOOL_TEXT_RIGHT = _aui.AUI_TBTOOL_TEXT_RIGHT
AUI_TBTOOL_TEXT_TOP = _aui.AUI_TBTOOL_TEXT_TOP
AUI_TBTOOL_TEXT_BOTTOM = _aui.AUI_TBTOOL_TEXT_BOTTOM
class AuiToolBarEvent(_core.NotifyEvent):
    """Proxy of C++ AuiToolBarEvent class"""
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    def __init__(self, *args, **kwargs): 
        """__init__(self, EventType command_type=wxEVT_NULL, int win_id=0) -> AuiToolBarEvent"""
        _aui.AuiToolBarEvent_swiginit(self,_aui.new_AuiToolBarEvent(*args, **kwargs))
    def IsDropDownClicked(*args, **kwargs):
        """IsDropDownClicked(self) -> bool"""
        return _aui.AuiToolBarEvent_IsDropDownClicked(*args, **kwargs)

    def SetDropDownClicked(*args, **kwargs):
        """SetDropDownClicked(self, bool c)"""
        return _aui.AuiToolBarEvent_SetDropDownClicked(*args, **kwargs)

    def GetClickPoint(*args, **kwargs):
        """GetClickPoint(self) -> Point"""
        return _aui.AuiToolBarEvent_GetClickPoint(*args, **kwargs)

    def SetClickPoint(*args, **kwargs):
        """SetClickPoint(self, Point p)"""
        return _aui.AuiToolBarEvent_SetClickPoint(*args, **kwargs)

    def GetItemRect(*args, **kwargs):
        """GetItemRect(self) -> Rect"""
        return _aui.AuiToolBarEvent_GetItemRect(*args, **kwargs)

    def SetItemRect(*args, **kwargs):
        """SetItemRect(self, Rect r)"""
        return _aui.AuiToolBarEvent_SetItemRect(*args, **kwargs)

    def GetToolId(*args, **kwargs):
        """GetToolId(self) -> int"""
        return _aui.AuiToolBarEvent_GetToolId(*args, **kwargs)

    def SetToolId(*args, **kwargs):
        """SetToolId(self, int id)"""
        return _aui.AuiToolBarEvent_SetToolId(*args, **kwargs)

_aui.AuiToolBarEvent_swigregister(AuiToolBarEvent)

class AuiToolBarItem(object):
    """Proxy of C++ AuiToolBarItem class"""
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    def __init__(self, *args): 
        """
        __init__(self) -> AuiToolBarItem
        __init__(self, AuiToolBarItem c) -> AuiToolBarItem
        """
        _aui.AuiToolBarItem_swiginit(self,_aui.new_AuiToolBarItem(*args))
    def Assign(*args, **kwargs):
        """Assign(self, AuiToolBarItem c)"""
        return _aui.AuiToolBarItem_Assign(*args, **kwargs)

    def SetWindow(*args, **kwargs):
        """SetWindow(self, Window w)"""
        return _aui.AuiToolBarItem_SetWindow(*args, **kwargs)

    def GetWindow(*args, **kwargs):
        """GetWindow(self) -> Window"""
        return _aui.AuiToolBarItem_GetWindow(*args, **kwargs)

    def SetId(*args, **kwargs):
        """SetId(self, int new_id)"""
        return _aui.AuiToolBarItem_SetId(*args, **kwargs)

    def GetId(*args, **kwargs):
        """GetId(self) -> int"""
        return _aui.AuiToolBarItem_GetId(*args, **kwargs)

    def SetKind(*args, **kwargs):
        """SetKind(self, int new_kind)"""
        return _aui.AuiToolBarItem_SetKind(*args, **kwargs)

    def GetKind(*args, **kwargs):
        """GetKind(self) -> int"""
        return _aui.AuiToolBarItem_GetKind(*args, **kwargs)

    def SetState(*args, **kwargs):
        """SetState(self, int new_state)"""
        return _aui.AuiToolBarItem_SetState(*args, **kwargs)

    def GetState(*args, **kwargs):
        """GetState(self) -> int"""
        return _aui.AuiToolBarItem_GetState(*args, **kwargs)

    def SetSizerItem(*args, **kwargs):
        """SetSizerItem(self, SizerItem s)"""
        return _aui.AuiToolBarItem_SetSizerItem(*args, **kwargs)

    def GetSizerItem(*args, **kwargs):
        """GetSizerItem(self) -> SizerItem"""
        return _aui.AuiToolBarItem_GetSizerItem(*args, **kwargs)

    def SetLabel(*args, **kwargs):
        """SetLabel(self, String s)"""
        return _aui.AuiToolBarItem_SetLabel(*args, **kwargs)

    def GetLabel(*args, **kwargs):
        """GetLabel(self) -> String"""
        return _aui.AuiToolBarItem_GetLabel(*args, **kwargs)

    def SetBitmap(*args, **kwargs):
        """SetBitmap(self, Bitmap bmp)"""
        return _aui.AuiToolBarItem_SetBitmap(*args, **kwargs)

    def GetBitmap(*args, **kwargs):
        """GetBitmap(self) -> Bitmap"""
        return _aui.AuiToolBarItem_GetBitmap(*args, **kwargs)

    def SetDisabledBitmap(*args, **kwargs):
        """SetDisabledBitmap(self, Bitmap bmp)"""
        return _aui.AuiToolBarItem_SetDisabledBitmap(*args, **kwargs)

    def GetDisabledBitmap(*args, **kwargs):
        """GetDisabledBitmap(self) -> Bitmap"""
        return _aui.AuiToolBarItem_GetDisabledBitmap(*args, **kwargs)

    def SetHoverBitmap(*args, **kwargs):
        """SetHoverBitmap(self, Bitmap bmp)"""
        return _aui.AuiToolBarItem_SetHoverBitmap(*args, **kwargs)

    def GetHoverBitmap(*args, **kwargs):
        """GetHoverBitmap(self) -> Bitmap"""
        return _aui.AuiToolBarItem_GetHoverBitmap(*args, **kwargs)

    def SetShortHelp(*args, **kwargs):
        """SetShortHelp(self, String s)"""
        return _aui.AuiToolBarItem_SetShortHelp(*args, **kwargs)

    def GetShortHelp(*args, **kwargs):
        """GetShortHelp(self) -> String"""
        return _aui.AuiToolBarItem_GetShortHelp(*args, **kwargs)

    def SetLongHelp(*args, **kwargs):
        """SetLongHelp(self, String s)"""
        return _aui.AuiToolBarItem_SetLongHelp(*args, **kwargs)

    def GetLongHelp(*args, **kwargs):
        """GetLongHelp(self) -> String"""
        return _aui.AuiToolBarItem_GetLongHelp(*args, **kwargs)

    def SetMinSize(*args, **kwargs):
        """SetMinSize(self, Size s)"""
        return _aui.AuiToolBarItem_SetMinSize(*args, **kwargs)

    def GetMinSize(*args, **kwargs):
        """GetMinSize(self) -> Size"""
        return _aui.AuiToolBarItem_GetMinSize(*args, **kwargs)

    def SetSpacerPixels(*args, **kwargs):
        """SetSpacerPixels(self, int s)"""
        return _aui.AuiToolBarItem_SetSpacerPixels(*args, **kwargs)

    def GetSpacerPixels(*args, **kwargs):
        """GetSpacerPixels(self) -> int"""
        return _aui.AuiToolBarItem_GetSpacerPixels(*args, **kwargs)

    def SetProportion(*args, **kwargs):
        """SetProportion(self, int p)"""
        return _aui.AuiToolBarItem_SetProportion(*args, **kwargs)

    def GetProportion(*args, **kwargs):
        """GetProportion(self) -> int"""
        return _aui.AuiToolBarItem_GetProportion(*args, **kwargs)

    def SetActive(*args, **kwargs):
        """SetActive(self, bool b)"""
        return _aui.AuiToolBarItem_SetActive(*args, **kwargs)

    def IsActive(*args, **kwargs):
        """IsActive(self) -> bool"""
        return _aui.AuiToolBarItem_IsActive(*args, **kwargs)

    def SetHasDropDown(*args, **kwargs):
        """SetHasDropDown(self, bool b)"""
        return _aui.AuiToolBarItem_SetHasDropDown(*args, **kwargs)

    def HasDropDown(*args, **kwargs):
        """HasDropDown(self) -> bool"""
        return _aui.AuiToolBarItem_HasDropDown(*args, **kwargs)

    def SetSticky(*args, **kwargs):
        """SetSticky(self, bool b)"""
        return _aui.AuiToolBarItem_SetSticky(*args, **kwargs)

    def IsSticky(*args, **kwargs):
        """IsSticky(self) -> bool"""
        return _aui.AuiToolBarItem_IsSticky(*args, **kwargs)

    def SetUserData(*args, **kwargs):
        """SetUserData(self, long l)"""
        return _aui.AuiToolBarItem_SetUserData(*args, **kwargs)

    def GetUserData(*args, **kwargs):
        """GetUserData(self) -> long"""
        return _aui.AuiToolBarItem_GetUserData(*args, **kwargs)

_aui.AuiToolBarItem_swigregister(AuiToolBarItem)

class AuiToolBarArt(object):
    """Proxy of C++ AuiToolBarArt class"""
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    def __init__(self): raise AttributeError, "No constructor defined"
    __repr__ = _swig_repr
    __swig_destroy__ = _aui.delete_AuiToolBarArt
    __del__ = lambda self : None;
    def Clone(*args, **kwargs):
        """Clone(self) -> AuiToolBarArt"""
        return _aui.AuiToolBarArt_Clone(*args, **kwargs)

    def SetFlags(*args, **kwargs):
        """SetFlags(self, int flags)"""
        return _aui.AuiToolBarArt_SetFlags(*args, **kwargs)

    def SetFont(*args, **kwargs):
        """SetFont(self, Font font)"""
        return _aui.AuiToolBarArt_SetFont(*args, **kwargs)

    def SetTextOrientation(*args, **kwargs):
        """SetTextOrientation(self, int orientation)"""
        return _aui.AuiToolBarArt_SetTextOrientation(*args, **kwargs)

    def DrawBackground(*args, **kwargs):
        """DrawBackground(self, DC dc, Window wnd, Rect rect)"""
        return _aui.AuiToolBarArt_DrawBackground(*args, **kwargs)

    def DrawLabel(*args, **kwargs):
        """DrawLabel(self, DC dc, Window wnd, AuiToolBarItem item, Rect rect)"""
        return _aui.AuiToolBarArt_DrawLabel(*args, **kwargs)

    def DrawButton(*args, **kwargs):
        """DrawButton(self, DC dc, Window wnd, AuiToolBarItem item, Rect rect)"""
        return _aui.AuiToolBarArt_DrawButton(*args, **kwargs)

    def DrawDropDownButton(*args, **kwargs):
        """DrawDropDownButton(self, DC dc, Window wnd, AuiToolBarItem item, Rect rect)"""
        return _aui.AuiToolBarArt_DrawDropDownButton(*args, **kwargs)

    def DrawControlLabel(*args, **kwargs):
        """DrawControlLabel(self, DC dc, Window wnd, AuiToolBarItem item, Rect rect)"""
        return _aui.AuiToolBarArt_DrawControlLabel(*args, **kwargs)

    def DrawSeparator(*args, **kwargs):
        """DrawSeparator(self, DC dc, Window wnd, Rect rect)"""
        return _aui.AuiToolBarArt_DrawSeparator(*args, **kwargs)

    def DrawGripper(*args, **kwargs):
        """DrawGripper(self, DC dc, Window wnd, Rect rect)"""
        return _aui.AuiToolBarArt_DrawGripper(*args, **kwargs)

    def DrawOverflowButton(*args, **kwargs):
        """DrawOverflowButton(self, DC dc, Window wnd, Rect rect, int state)"""
        return _aui.AuiToolBarArt_DrawOverflowButton(*args, **kwargs)

    def GetLabelSize(*args, **kwargs):
        """GetLabelSize(self, DC dc, Window wnd, AuiToolBarItem item) -> Size"""
        return _aui.AuiToolBarArt_GetLabelSize(*args, **kwargs)

    def GetToolSize(*args, **kwargs):
        """GetToolSize(self, DC dc, Window wnd, AuiToolBarItem item) -> Size"""
        return _aui.AuiToolBarArt_GetToolSize(*args, **kwargs)

    def GetElementSize(*args, **kwargs):
        """GetElementSize(self, int element_id) -> int"""
        return _aui.AuiToolBarArt_GetElementSize(*args, **kwargs)

    def SetElementSize(*args, **kwargs):
        """SetElementSize(self, int element_id, int size)"""
        return _aui.AuiToolBarArt_SetElementSize(*args, **kwargs)

    def ShowDropDown(*args, **kwargs):
        """ShowDropDown(self, Window wnd, wxAuiToolBarItemArray items) -> int"""
        return _aui.AuiToolBarArt_ShowDropDown(*args, **kwargs)

_aui.AuiToolBarArt_swigregister(AuiToolBarArt)

class AuiDefaultToolBarArt(AuiToolBarArt):
    """Proxy of C++ AuiDefaultToolBarArt class"""
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    def __init__(self, *args, **kwargs): 
        """__init__(self) -> AuiDefaultToolBarArt"""
        _aui.AuiDefaultToolBarArt_swiginit(self,_aui.new_AuiDefaultToolBarArt(*args, **kwargs))
    __swig_destroy__ = _aui.delete_AuiDefaultToolBarArt
    __del__ = lambda self : None;
_aui.AuiDefaultToolBarArt_swigregister(AuiDefaultToolBarArt)

class AuiToolBar(_core.Control):
    """Proxy of C++ AuiToolBar class"""
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    def __init__(self, *args, **kwargs): 
        """
        __init__(self, Window parent, int id=-1, Point position=DefaultPosition, 
            Size size=DefaultSize, long style=AUI_TB_DEFAULT_STYLE) -> AuiToolBar
        """
        _aui.AuiToolBar_swiginit(self,_aui.new_AuiToolBar(*args, **kwargs))
    __swig_destroy__ = _aui.delete_AuiToolBar
    __del__ = lambda self : None;
    def SetArtProvider(*args, **kwargs):
        """SetArtProvider(self, AuiToolBarArt art)"""
        return _aui.AuiToolBar_SetArtProvider(*args, **kwargs)

    def GetArtProvider(*args, **kwargs):
        """GetArtProvider(self) -> AuiToolBarArt"""
        return _aui.AuiToolBar_GetArtProvider(*args, **kwargs)

    def AddTool(*args):
        """
        AddTool(self, int tool_id, String label, Bitmap bitmap, String short_help_string=wxEmptyString, 
            int kind=ITEM_NORMAL)
        AddTool(self, int tool_id, String label, Bitmap bitmap, Bitmap disabled_bitmap, 
            int kind, String short_help_string, 
            String long_help_string, Object client_data)
        AddTool(self, int tool_id, Bitmap bitmap, Bitmap disabled_bitmap, 
            bool toggle=False, Object client_data=None, 
            String short_help_string=wxEmptyString, String long_help_string=wxEmptyString)
        """
        return _aui.AuiToolBar_AddTool(*args)

    def AddLabel(*args, **kwargs):
        """AddLabel(self, int tool_id, String label=wxEmptyString, int width=-1)"""
        return _aui.AuiToolBar_AddLabel(*args, **kwargs)

    def AddControl(*args, **kwargs):
        """AddControl(self, Control control, String label=wxEmptyString)"""
        return _aui.AuiToolBar_AddControl(*args, **kwargs)

    def AddSeparator(*args, **kwargs):
        """AddSeparator(self)"""
        return _aui.AuiToolBar_AddSeparator(*args, **kwargs)

    def AddSpacer(*args, **kwargs):
        """AddSpacer(self, int pixels)"""
        return _aui.AuiToolBar_AddSpacer(*args, **kwargs)

    def AddStretchSpacer(*args, **kwargs):
        """AddStretchSpacer(self, int proportion=1)"""
        return _aui.AuiToolBar_AddStretchSpacer(*args, **kwargs)

    def Realize(*args, **kwargs):
        """Realize(self) -> bool"""
        return _aui.AuiToolBar_Realize(*args, **kwargs)

    def FindControl(*args, **kwargs):
        """FindControl(self, int window_id) -> Control"""
        return _aui.AuiToolBar_FindControl(*args, **kwargs)

    def FindToolByPosition(*args, **kwargs):
        """FindToolByPosition(self, int x, int y) -> AuiToolBarItem"""
        return _aui.AuiToolBar_FindToolByPosition(*args, **kwargs)

    def FindToolByIndex(*args, **kwargs):
        """FindToolByIndex(self, int idx) -> AuiToolBarItem"""
        return _aui.AuiToolBar_FindToolByIndex(*args, **kwargs)

    def FindTool(*args, **kwargs):
        """FindTool(self, int tool_id) -> AuiToolBarItem"""
        return _aui.AuiToolBar_FindTool(*args, **kwargs)

    def ClearTools(*args, **kwargs):
        """ClearTools(self)"""
        return _aui.AuiToolBar_ClearTools(*args, **kwargs)

    def Clear(*args, **kwargs):
        """Clear(self)"""
        return _aui.AuiToolBar_Clear(*args, **kwargs)

    def DeleteTool(*args, **kwargs):
        """DeleteTool(self, int tool_id) -> bool"""
        return _aui.AuiToolBar_DeleteTool(*args, **kwargs)

    def DeleteByIndex(*args, **kwargs):
        """DeleteByIndex(self, int tool_id) -> bool"""
        return _aui.AuiToolBar_DeleteByIndex(*args, **kwargs)

    def GetToolCount(*args, **kwargs):
        """GetToolCount(self) -> size_t"""
        return _aui.AuiToolBar_GetToolCount(*args, **kwargs)

    def GetToolPos(*args, **kwargs):
        """GetToolPos(self, int tool_id) -> int"""
        return _aui.AuiToolBar_GetToolPos(*args, **kwargs)

    def GetToolIndex(*args, **kwargs):
        """GetToolIndex(self, int tool_id) -> int"""
        return _aui.AuiToolBar_GetToolIndex(*args, **kwargs)

    def GetToolFits(*args, **kwargs):
        """GetToolFits(self, int tool_id) -> bool"""
        return _aui.AuiToolBar_GetToolFits(*args, **kwargs)

    def GetToolRect(*args, **kwargs):
        """GetToolRect(self, int tool_id) -> Rect"""
        return _aui.AuiToolBar_GetToolRect(*args, **kwargs)

    def GetToolFitsByIndex(*args, **kwargs):
        """GetToolFitsByIndex(self, int tool_id) -> bool"""
        return _aui.AuiToolBar_GetToolFitsByIndex(*args, **kwargs)

    def GetToolBarFits(*args, **kwargs):
        """GetToolBarFits(self) -> bool"""
        return _aui.AuiToolBar_GetToolBarFits(*args, **kwargs)

    def SetMargins(*args):
        """
        SetMargins(self, Size size)
        SetMargins(self, int x, int y)
        SetMargins(self, int left, int right, int top, int bottom)
        """
        return _aui.AuiToolBar_SetMargins(*args)

    def SetToolBitmapSize(*args, **kwargs):
        """SetToolBitmapSize(self, Size size)"""
        return _aui.AuiToolBar_SetToolBitmapSize(*args, **kwargs)

    def GetToolBitmapSize(*args, **kwargs):
        """GetToolBitmapSize(self) -> Size"""
        return _aui.AuiToolBar_GetToolBitmapSize(*args, **kwargs)

    def GetOverflowVisible(*args, **kwargs):
        """GetOverflowVisible(self) -> bool"""
        return _aui.AuiToolBar_GetOverflowVisible(*args, **kwargs)

    def SetOverflowVisible(*args, **kwargs):
        """SetOverflowVisible(self, bool visible)"""
        return _aui.AuiToolBar_SetOverflowVisible(*args, **kwargs)

    def GetGripperVisible(*args, **kwargs):
        """GetGripperVisible(self) -> bool"""
        return _aui.AuiToolBar_GetGripperVisible(*args, **kwargs)

    def SetGripperVisible(*args, **kwargs):
        """SetGripperVisible(self, bool visible)"""
        return _aui.AuiToolBar_SetGripperVisible(*args, **kwargs)

    def ToggleTool(*args, **kwargs):
        """ToggleTool(self, int tool_id, bool state)"""
        return _aui.AuiToolBar_ToggleTool(*args, **kwargs)

    def GetToolToggled(*args, **kwargs):
        """GetToolToggled(self, int tool_id) -> bool"""
        return _aui.AuiToolBar_GetToolToggled(*args, **kwargs)

    def EnableTool(*args, **kwargs):
        """EnableTool(self, int tool_id, bool state)"""
        return _aui.AuiToolBar_EnableTool(*args, **kwargs)

    def GetToolEnabled(*args, **kwargs):
        """GetToolEnabled(self, int tool_id) -> bool"""
        return _aui.AuiToolBar_GetToolEnabled(*args, **kwargs)

    def SetToolDropDown(*args, **kwargs):
        """SetToolDropDown(self, int tool_id, bool dropdown)"""
        return _aui.AuiToolBar_SetToolDropDown(*args, **kwargs)

    def GetToolDropDown(*args, **kwargs):
        """GetToolDropDown(self, int tool_id) -> bool"""
        return _aui.AuiToolBar_GetToolDropDown(*args, **kwargs)

    def SetToolBorderPadding(*args, **kwargs):
        """SetToolBorderPadding(self, int padding)"""
        return _aui.AuiToolBar_SetToolBorderPadding(*args, **kwargs)

    def GetToolBorderPadding(*args, **kwargs):
        """GetToolBorderPadding(self) -> int"""
        return _aui.AuiToolBar_GetToolBorderPadding(*args, **kwargs)

    def SetToolTextOrientation(*args, **kwargs):
        """SetToolTextOrientation(self, int orientation)"""
        return _aui.AuiToolBar_SetToolTextOrientation(*args, **kwargs)

    def GetToolTextOrientation(*args, **kwargs):
        """GetToolTextOrientation(self) -> int"""
        return _aui.AuiToolBar_GetToolTextOrientation(*args, **kwargs)

    def SetToolPacking(*args, **kwargs):
        """SetToolPacking(self, int packing)"""
        return _aui.AuiToolBar_SetToolPacking(*args, **kwargs)

    def GetToolPacking(*args, **kwargs):
        """GetToolPacking(self) -> int"""
        return _aui.AuiToolBar_GetToolPacking(*args, **kwargs)

    def SetToolProportion(*args, **kwargs):
        """SetToolProportion(self, int tool_id, int proportion)"""
        return _aui.AuiToolBar_SetToolProportion(*args, **kwargs)

    def GetToolProportion(*args, **kwargs):
        """GetToolProportion(self, int tool_id) -> int"""
        return _aui.AuiToolBar_GetToolProportion(*args, **kwargs)

    def SetToolSeparation(*args, **kwargs):
        """SetToolSeparation(self, int separation)"""
        return _aui.AuiToolBar_SetToolSeparation(*args, **kwargs)

    def GetToolSeparation(*args, **kwargs):
        """GetToolSeparation(self) -> int"""
        return _aui.AuiToolBar_GetToolSeparation(*args, **kwargs)

    def SetToolSticky(*args, **kwargs):
        """SetToolSticky(self, int tool_id, bool sticky)"""
        return _aui.AuiToolBar_SetToolSticky(*args, **kwargs)

    def GetToolSticky(*args, **kwargs):
        """GetToolSticky(self, int tool_id) -> bool"""
        return _aui.AuiToolBar_GetToolSticky(*args, **kwargs)

    def GetToolLabel(*args, **kwargs):
        """GetToolLabel(self, int tool_id) -> String"""
        return _aui.AuiToolBar_GetToolLabel(*args, **kwargs)

    def SetToolLabel(*args, **kwargs):
        """SetToolLabel(self, int tool_id, String label)"""
        return _aui.AuiToolBar_SetToolLabel(*args, **kwargs)

    def GetToolBitmap(*args, **kwargs):
        """GetToolBitmap(self, int tool_id) -> Bitmap"""
        return _aui.AuiToolBar_GetToolBitmap(*args, **kwargs)

    def SetToolBitmap(*args, **kwargs):
        """SetToolBitmap(self, int tool_id, Bitmap bitmap)"""
        return _aui.AuiToolBar_SetToolBitmap(*args, **kwargs)

    def GetToolShortHelp(*args, **kwargs):
        """GetToolShortHelp(self, int tool_id) -> String"""
        return _aui.AuiToolBar_GetToolShortHelp(*args, **kwargs)

    def SetToolShortHelp(*args, **kwargs):
        """SetToolShortHelp(self, int tool_id, String help_string)"""
        return _aui.AuiToolBar_SetToolShortHelp(*args, **kwargs)

    def GetToolLongHelp(*args, **kwargs):
        """GetToolLongHelp(self, int tool_id) -> String"""
        return _aui.AuiToolBar_GetToolLongHelp(*args, **kwargs)

    def SetToolLongHelp(*args, **kwargs):
        """SetToolLongHelp(self, int tool_id, String help_string)"""
        return _aui.AuiToolBar_SetToolLongHelp(*args, **kwargs)

    def SetCustomOverflowItems(*args, **kwargs):
        """SetCustomOverflowItems(self, wxAuiToolBarItemArray prepend, wxAuiToolBarItemArray append)"""
        return _aui.AuiToolBar_SetCustomOverflowItems(*args, **kwargs)

_aui.AuiToolBar_swigregister(AuiToolBar)

wxEVT_COMMAND_AUITOOLBAR_TOOL_DROPDOWN = _aui.wxEVT_COMMAND_AUITOOLBAR_TOOL_DROPDOWN
wxEVT_COMMAND_AUITOOLBAR_OVERFLOW_CLICK = _aui.wxEVT_COMMAND_AUITOOLBAR_OVERFLOW_CLICK
wxEVT_COMMAND_AUITOOLBAR_RIGHT_CLICK = _aui.wxEVT_COMMAND_AUITOOLBAR_RIGHT_CLICK
wxEVT_COMMAND_AUITOOLBAR_MIDDLE_CLICK = _aui.wxEVT_COMMAND_AUITOOLBAR_MIDDLE_CLICK
wxEVT_COMMAND_AUITOOLBAR_BEGIN_DRAG = _aui.wxEVT_COMMAND_AUITOOLBAR_BEGIN_DRAG
EVT_AUITOOLBAR_TOOL_DROPDOWN = wx.PyEventBinder( wxEVT_COMMAND_AUITOOLBAR_TOOL_DROPDOWN, 1 )
EVT_AUITOOLBAR_OVERFLOW_CLICK = wx.PyEventBinder( wxEVT_COMMAND_AUITOOLBAR_OVERFLOW_CLICK, 1 )
EVT_AUITOOLBAR_RIGHT_CLICK = wx.PyEventBinder( wxEVT_COMMAND_AUITOOLBAR_RIGHT_CLICK, 1 )
EVT_AUITOOLBAR_MIDDLE_CLICK = wx.PyEventBinder( wxEVT_COMMAND_AUITOOLBAR_MIDDLE_CLICK, 1 )
EVT_AUITOOLBAR_BEGIN_DRAG = wx.PyEventBinder( wxEVT_COMMAND_AUITOOLBAR_BEGIN_DRAG, 1 )

class PyAuiDockArt(AuiDefaultDockArt):
    """
    This version of the `AuiDockArt` class has been instrumented to be
    subclassable in Python and to reflect all calls to the C++ base class
    methods to the Python methods implemented in the derived class.
    """
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    def __init__(self, *args, **kwargs): 
        """
        __init__(self) -> PyAuiDockArt

        This version of the `AuiDockArt` class has been instrumented to be
        subclassable in Python and to reflect all calls to the C++ base class
        methods to the Python methods implemented in the derived class.
        """
        _aui.PyAuiDockArt_swiginit(self,_aui.new_PyAuiDockArt(*args, **kwargs))
        PyAuiDockArt._setCallbackInfo(self, self, PyAuiDockArt)

    def _setCallbackInfo(*args, **kwargs):
        """_setCallbackInfo(self, PyObject self, PyObject _class)"""
        return _aui.PyAuiDockArt__setCallbackInfo(*args, **kwargs)

_aui.PyAuiDockArt_swigregister(PyAuiDockArt)

class PyAuiTabArt(AuiDefaultTabArt):
    """
    This version of the `TabArt` class has been instrumented to be
    subclassable in Python and to reflect all calls to the C++ base class
    methods to the Python methods implemented in the derived class.
    """
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    def __init__(self, *args, **kwargs): 
        """
        __init__(self) -> PyAuiTabArt

        This version of the `TabArt` class has been instrumented to be
        subclassable in Python and to reflect all calls to the C++ base class
        methods to the Python methods implemented in the derived class.
        """
        _aui.PyAuiTabArt_swiginit(self,_aui.new_PyAuiTabArt(*args, **kwargs))
        PyAuiTabArt._setCallbackInfo(self, self, PyAuiTabArt)

    def _setCallbackInfo(*args, **kwargs):
        """_setCallbackInfo(self, PyObject self, PyObject _class)"""
        return _aui.PyAuiTabArt__setCallbackInfo(*args, **kwargs)

    def GetNormalFont(*args, **kwargs):
        """GetNormalFont(self) -> Font"""
        return _aui.PyAuiTabArt_GetNormalFont(*args, **kwargs)

    def GetSelectedFont(*args, **kwargs):
        """GetSelectedFont(self) -> Font"""
        return _aui.PyAuiTabArt_GetSelectedFont(*args, **kwargs)

    def GetMeasuringFont(*args, **kwargs):
        """GetMeasuringFont(self) -> Font"""
        return _aui.PyAuiTabArt_GetMeasuringFont(*args, **kwargs)

    def GetFlags(*args, **kwargs):
        """GetFlags(self) -> int"""
        return _aui.PyAuiTabArt_GetFlags(*args, **kwargs)

_aui.PyAuiTabArt_swigregister(PyAuiTabArt)



