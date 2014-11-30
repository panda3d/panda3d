__author__  = "E. A. Tacao <e.a.tacao |at| estadao.com.br>"
__date__    = "12 Fev 2006, 22:00 GMT-03:00"
__version__ = "0.03"
__doc__     = """
ButtonTreeCtrlPanel is a widget where one can place check buttons, tri-state
check buttons, radio buttons, both, and the ability to display them
hierarchically.


About:

ButtonTreeCtrlPanel is distributed under the wxWidgets license.

For all kind of problems, requests, enhancements, bug reports, etc,
please drop me an e-mail.

For updates please visit <http://j.domaindlx.com/elements28/wxpython/>.
"""

import cStringIO

import wx
from wx.lib.newevent import NewEvent

#----------------------------------------------------------------------------

(ButtonTreeCtrlPanelEvent, EVT_BUTTONTREECTRLPANEL) = NewEvent()
EVT_CHANGED = EVT_BUTTONTREECTRLPANEL

#----------------------------------------------------------------------------

class ButtonTreeCtrlPanel(wx.Panel):
    def __init__(self, parent, id=wx.ID_ANY, pos=wx.DefaultPosition,
                 size=wx.DefaultSize, style=wx.WANTS_CHARS):
        wx.Panel.__init__(self, parent, id, pos, size, style)

        self.tree = wx.TreeCtrl(self, style=wx.TR_NO_LINES|wx.TR_HIDE_ROOT)

        il = self.il = wx.ImageList(16, 16)
        self.tree.SetImageList(il)

        for bl in ["checkbox_checked", "checkbox_unchecked", "checkbox_tri",
                   "radiobox_checked", "radiobox_unchecked"]:
            bitmap = getattr(self.__class__, bl).GetBitmap()
            setattr(self, bl, il.Add(bitmap))

        bmp = wx.ArtProvider.GetBitmap(wx.ART_FOLDER, wx.ART_TOOLBAR, (16, 16))
        self.empty_bitmap = il.Add(bmp)

        self.root = self.tree.AddRoot("Root Item for ButtonTreeCtrlPanel")

        self.Bind(wx.EVT_SIZE, self.OnSize)
        self.tree.Bind(wx.EVT_LEFT_DCLICK, self.OnLeftClicks)
        self.tree.Bind(wx.EVT_LEFT_DOWN, self.OnLeftClicks)
        self.tree.Bind(wx.EVT_RIGHT_DOWN, self.OnRightClick)

        self.allitems = []

        wx.CallAfter(self.OnSize)


    def _doLogicTest(self, style, value, item):
        if style in [wx.CHK_2STATE, wx.CHK_3STATE]:
            n = [self.checkbox_unchecked, self.checkbox_checked, \
                 self.checkbox_tri][value]

            self.tree.SetPyData(item, (value, style))
            self.tree.SetItemImage(item, n, wx.TreeItemIcon_Normal)

        elif style == wx.RB_SINGLE:
            if value:
                parent = self.tree.GetItemParent(item)
                (child, cookie) = self.tree.GetFirstChild(parent)

                if self.tree.GetPyData(child):
                    self.tree.SetPyData(child, (False, wx.RB_SINGLE))
                    self.tree.SetItemImage(child, self.radiobox_unchecked, \
                                           wx.TreeItemIcon_Normal)

                for x in range(1, self.tree.GetChildrenCount(parent, False)):
                    (child, cookie) = self.tree.GetNextChild(parent, cookie)

                    if self.tree.GetPyData(child):
                        self.tree.SetPyData(child, (False, wx.RB_SINGLE))
                        self.tree.SetItemImage(child, self.radiobox_unchecked, \
                                               wx.TreeItemIcon_Normal)

                self.tree.SetPyData(item, (True, wx.RB_SINGLE))
                self.tree.SetItemImage(item, self.radiobox_checked, \
                                       wx.TreeItemIcon_Normal)

            else:
                self.tree.SetPyData(item, (False, wx.RB_SINGLE))
                self.tree.SetItemImage(item, self.radiobox_unchecked, \
                                       wx.TreeItemIcon_Normal)


    def _getItems(self, parent=None, value=None):
        if not parent:
            parent = self.root
        cil = []
        (child, cookie) = self.tree.GetFirstChild(parent)
        if child.IsOk():
            d = self.tree.GetPyData(child)
            if value is None or (d and d[0] == value):
                cil.append(child)
            for x in range(1, self.tree.GetChildrenCount(parent, False)):
                (child, cookie) = self.tree.GetNextChild(parent, cookie)
                if child.IsOk():
                    d = self.tree.GetPyData(child)
                    if value is None or (d and d[0] == value):
                        cil.append(child)
        return cil


    def AddItem(self, label, bmp=None, parent=None, style=None, value=False):
        v = None

        if bmp:
            n = self.il.Add(bmp)
        if not parent:
            parent = self.root
        if style is not None:
            v = (value, style)

        this_item = self.tree.AppendItem(parent, label)
        self.tree.SetPyData(this_item, v)

        if v:
            self._doLogicTest(style, value, this_item)
        else:
            if bmp is None:
                bmp = self.empty_bitmap
            else:
                bmp = self.il.Add(bmp)

            self.tree.SetItemImage(this_item, bmp, wx.TreeItemIcon_Normal)

        self.allitems.append(this_item)
        [self.tree.Expand(x) for x in self.allitems]

        return this_item


    def ExpandItem(self, item):
        self.tree.Expand(item)


    def CollapseItem(self, item):
        self.tree.Collapse(item)


    def EnsureFirstVisible(self):
        (child, cookie) = self.tree.GetFirstChild(self.root)
        if child.IsOk():
            self.tree.SelectItem(child)
            self.tree.EnsureVisible(child)


    def SetItemValue(self, item, value):
        data = self.tree.GetPyData(item)
        if data:
            self._doLogicTest(data[1], value, item)


    def GetItemValue(self, item):
        data = self.tree.GetPyData(item)
        if data:
            return data[0]
        else:
            return None


    def GetItemByLabel(self, label, parent=None):
        r = None
        for item in self._getItems(parent):
            if self.tree.GetItemText(item) == label:
                r = item; break
        return r


    def GetAllItems(self):
        return self.allitems


    def GetRootItems(self):
        cil = []
        for x in range(0, len(self.allitems)):
            d = self.tree.GetPyData(self.allitems[x])
            if not d:
                cil.append(self.allitems[x])
        return cil


    def GetStringRootItems(self):
        return [self.tree.GetItemText(x) for x in self.GetRootItems]


    def GetItemsUnchecked(self, parent=None):
        return self._getItems(parent, 0)


    def GetItemsChecked(self, parent=None):
        return self._getItems(parent, 1)


    def GetItemsTri(self, parent=None):
        return self._getItems(parent, 2)


    def GetStringItemsUnchecked(self, parent=None):
        return [self.tree.GetItemText(x) \
                for x in self.GetItemsUnchecked(parent)]


    def GetStringItemsChecked(self, parent=None):
        return [self.tree.GetItemText(x) for x in self.GetItemsChecked(parent)]


    def GetStringItemsTri(self, parent=None):
        return [self.tree.GetItemText(x) for x in self.GetItemsTri(parent)]


    def OnRightClick(self, evt):
        item, flags = self.tree.HitTest(evt.GetPosition())
        self.tree.SelectItem(item)


    def OnLeftClicks(self, evt):
        item, flags = self.tree.HitTest(evt.GetPosition())
        if item:
            text, data = self.tree.GetItemText(item), self.tree.GetPyData(item)
            if data:
                style = data[1]
                if style == wx.CHK_2STATE:
                    value = not data[0]
                elif style == wx.CHK_3STATE:
                    value = data[0] + 1
                    if value == 3: value = 0
                else:
                    value = True

                self._doLogicTest(style, value, item)

                if value <> data[0]:
                    nevt = ButtonTreeCtrlPanelEvent(obj=self, id=self.GetId(),
                                                    item=item, val=value)
                    wx.PostEvent(self, nevt)

        evt.Skip()


    def OnSize(self, evt=None):
        self.tree.SetSize(self.GetClientSize())

# # Images generated by encode_bitmaps.py -----------------------------
from wx.lib.embeddedimage import PyEmbeddedImage

ButtonTreeCtrlPanel.checkbox_unchecked = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAABHNCSVQICAgIfAhkiAAAAEFJ"
    "REFUOI3tkzsOACAUwsrT+9/Yz6yDieJkZKfpAFIknITVBjJAq6XtFhVJ9wxm6iqzrW3wAU8A"
    "hiGdTNo2kHvnDr+YDCrzE+JlAAAAAElFTkSuQmCC")

ButtonTreeCtrlPanel.radiobox_checked = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAABHNCSVQICAgIfAhkiAAAAHFJ"
    "REFUOI2tUtESgCAIA+3//1jpqW7R5tkRb8o2GODeulWildhmdqhEzBH49tad4TxbyMQXIQk9"
    "BJCcgSpHZ8DaVRZugasCAmOOYJXxT24BQau5lNcoBdCK8m8mtqAILE87YJ7VHP49pJXQ9il/"
    "jfIaT195QDiwOHL5AAAAAElFTkSuQmCC")

ButtonTreeCtrlPanel.radiobox_unchecked = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAABHNCSVQICAgIfAhkiAAAAGdJ"
    "REFUOI3NkksSgDAIQ4F6/xtru9LBmHTq4EJ2Hchr+LhHs0pESW1mm0r0Y+/57dGc1Tm2gMKH"
    "AEA3QBZjocrRGTC7qoULcP6gCnMuuylv4UcA1h8GmxN1wCAK/O0hzUDLp/w2ylsY3w4wQW9/"
    "cegAAAAASUVORK5CYII=")

ButtonTreeCtrlPanel.checkbox_checked = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAABHNCSVQICAgIfAhkiAAAAGdJ"
    "REFUOI2tk1EOgDAIQ1vm/W+s82uJqbAxkW9eU6CQ1lApK9EADgDo19l3QVrjfw5UdVbqNu0g"
    "GjMlMNvRS0CbVwt2HQzoCUf7CUfIwK6ANq8u4zoYUOas4QgZGJAgfYl0OcqsvvMNP8koKiUm"
    "7JsAAAAASUVORK5CYII=")

ButtonTreeCtrlPanel.checkbox_tri = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAABHNCSVQICAgIfAhkiAAAAHBJ"
    "REFUOI2tk0EOgDAIBJfqq9Sj+mj1aP1We2piCCCKnJnN0GyJUofIpBANoAeAaRzKW/DMF/1n"
    "wFOt4bZug2PfxDNdARosBvBlC1YNGnSH52UV30c9wQOLAXzZglWDBj3BaoAXBliRvlQ6XGWK"
    "fucKTYUl4c5UOHYAAAAASUVORK5CYII=")

#
##
### eof
