###############################################################################
# Name: elistctrl.py                                                          #
# Purpose: Base ListCtrl                                                      #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2010 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
Editra Control Library: EListCtrl

Class EBaseListCtrl:
Base Report mode ListCtrl class that highlights alternate rows

Class ECheckListCtrl:
Child class of L{EBaseListCtrl} that also provides CheckBoxes in the first
column of the control.

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: elistctrl.py 67330 2011-03-29 02:48:06Z CJP $"
__revision__ = "$Revision: 67330 $"

__all__ = ["EBaseListCtrl", "ECheckListCtrl", "EEditListCtrl", 
           "EToggleEditListCtrl"]

#--------------------------------------------------------------------------#
# Dependencies
import wx
import wx.lib.mixins.listctrl as listmix

# Local Imports
import elistmix

#--------------------------------------------------------------------------#

class EBaseListCtrl(elistmix.ListRowHighlighter,
                    listmix.ListCtrlAutoWidthMixin,
                    wx.ListCtrl):
    """Base listctrl class that provides automatic row highlighting"""
    def __init__(self, parent, _id=wx.ID_ANY,
                 pos=wx.DefaultPosition, size=wx.DefaultSize,
                 style=wx.LC_REPORT, validator=wx.DefaultValidator,
                 name="EListCtrl"):
        wx.ListCtrl.__init__(self, parent, _id, pos, size,
                             style, validator, name)
        elistmix.ListRowHighlighter.__init__(self)
        listmix.ListCtrlAutoWidthMixin.__init__(self)

    def EnableRow(self, idx, enable=True):
        """Enable/Disable a row in the ListCtrl
        @param idx: row index

        """
        state = 0
        txtcolour = wx.SYS_COLOUR_LISTBOXTEXT
        if not enable:
            state = wx.LIST_STATE_DISABLED
            txtcolour = wx.SYS_COLOUR_GRAYTEXT
        self.SetItemState(idx, state, wx.LIST_STATE_DONTCARE)
        colour = wx.SystemSettings.GetColour(txtcolour)
        self.SetItemTextColour(idx, colour)

    def GetRowData(self, idx):
        """Get the values from each cell in the given row
        @param idx: row index
        @return: tuple

        """
        data = list()
        if idx >= 0 and idx < self.GetItemCount():
            for col in range(self.GetColumnCount()):
                item = self.GetItem(idx, col)
                data.append(item.Text)
        return tuple(data)

    def GetSelections(self):
        """Get a list of all the selected items in the list
        @return: list of ints

        """
        items = [ idx for idx in range(self.GetItemCount())
                  if self.IsSelected(idx) ]
        return items

    def HasSelection(self):
        """Are any items selected in the list"""
        return bool(len(self.GetSelections()))

class ECheckListCtrl(listmix.CheckListCtrlMixin,
                     EBaseListCtrl):
     """ListCtrl with CheckBoxes in the first column"""
     def __init__(self, *args, **kwargs):
         EBaseListCtrl.__init__(self, *args, **kwargs)
         listmix.CheckListCtrlMixin.__init__(self)

class EEditListCtrl(listmix.TextEditMixin,
                    EBaseListCtrl):
    """ListCtrl with Editable cells"""
    def __init__(self, *args, **kwargs):
        EBaseListCtrl.__init__(self, *args, **kwargs)
        listmix.TextEditMixin.__init__(self)

class EToggleEditListCtrl(listmix.CheckListCtrlMixin,
                          listmix.TextEditMixin,
                          EBaseListCtrl):
    """ListCtrl with Editable cells and images that can be toggled in the
    the first column.

    """
    def __init__(self, *args, **kwargs):
        EBaseListCtrl.__init__(self, *args, **kwargs)
        listmix.TextEditMixin.__init__(self)
        listmix.CheckListCtrlMixin.__init__(self)
        self.Unbind(wx.EVT_LEFT_DCLICK)

    def GetCheckedItems(self):
        """Get the list of checked indexes"""
        count = self.GetItemCount()
        return [item for item in range(count) if self.IsChecked(item)]

    def SetCheckedBitmap(self, bmp):
        """Set the bitmap to use for the Checked state
        @param bmp: wx.Bitmap

        """
        assert isinstance(bmp, wx.Bitmap) and bmp.IsOk()
        imgl = self.GetImageList(wx.IMAGE_LIST_SMALL)
        imgl.Replace(self.check_image, bmp)

    def SetUnCheckedBitmap(self, bmp):
        """Set the bitmap to use for the un-Checked state
        @param bmp: wx.Bitmap

        """
        assert isinstance(bmp, wx.Bitmap) and bmp.IsOk()
        imgl = self.GetImageList(wx.IMAGE_LIST_SMALL)
        imgl.Replace(self.uncheck_image, bmp)
