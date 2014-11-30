###############################################################################
# Name: FileInfo.py                                                           #
# Purpose: Display information about files/folders                            #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FileInfo.py

Dialog for displaying file information.

Displays information on:
  * Filename and Path
  * File Size
  * Read/Write/Execute permissions
  * Creation/Modification times

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: infodlg.py 66025 2010-11-05 19:18:08Z CJP $"
__revision__ = "$Revision: 66025 $"

__all__ = ["FileInfoDlg", "CalcSize", "GetFileType"]

#--------------------------------------------------------------------------#
# Imports
import os
import time
import stat
import mimetypes
import wx

#--------------------------------------------------------------------------#
# Globals

_ = wx.GetTranslation

PERM_MAP = { '0' : '---', '1' : '--x', '2' : '-w-', '3' : '-wx',
             '4' : 'r--', '5' : 'r-x', '6' : 'rw-', '7' : 'rwx'}

#--------------------------------------------------------------------------#

class FileInfoDlg(wx.MiniFrame):
    """Dialog for displaying information about a file"""
    def __init__(self, parent, fname='', ftype=None, bmp=wx.NullBitmap):
        """Create the dialog with the information of the given file
        @param parent: Parent Window
        @keyword fname: File Path
        @keyword ftype: Filetype label (leave None to automatically determine)
        @keyword bmp: wxBitmap

        """
        self._fname = fname.split(os.path.sep)[-1]
        super(FileInfoDlg, self).__init__(parent,
                                          title="%s  %s" % (self._fname, _("Info")),
                                          style=wx.DEFAULT_DIALOG_STYLE)

        # Attributes
        self._file = fname
        self._ftype = ftype
        self.panel = wx.Panel(self)
        if bmp.IsNull():
            bmp = wx.ArtProvider.GetBitmap(wx.ART_INFORMATION, wx.ART_CMN_DIALOG)
        self._bmp = wx.StaticBitmap(self.panel, bitmap=bmp)
        self._ftxt = wx.StaticText(self.panel)

        try:
            fstat = os.stat(fname)
            perm = oct(stat.S_IMODE(fstat[stat.ST_MODE])).lstrip('0')
            permstr = ''
            for bit in perm:
                permstr += (PERM_MAP.get(bit, '---') + " ")
            self._fstat = dict(mtime=time.asctime(time.localtime(fstat[stat.ST_MTIME])),
                               ctime=time.asctime(time.localtime(fstat[stat.ST_CTIME])),
                               size=CalcSize(fstat[stat.ST_SIZE]),
                               perm=permstr)
        except Exception, msg:
            self.__DoErrorLayout(str(msg))
        else:
            self.__DoLayout()

        self.panel.SetAutoLayout(True)
        fsizer = wx.BoxSizer(wx.VERTICAL)
        fsizer.Add(self.panel, 1, wx.EXPAND)
        self.SetSizer(fsizer)
        self.SetAutoLayout(True)
        self.SetInitialSize()

        # Event Handlers
        self.Bind(wx.EVT_CLOSE, self.OnClose)

    def __DoErrorLayout(self, msg):
        """Set the dialogs display up for when an error happened in
        the stat call.

        """
        # Top Info
        top = wx.BoxSizer(wx.HORIZONTAL)
        head = wx.BoxSizer(wx.VERTICAL)
        err = wx.ArtProvider.GetBitmap(wx.ART_ERROR, wx.ART_CMN_DIALOG)
        bmp = wx.StaticBitmap(self.panel, bitmap=err)
        lbl = wx.StaticText(self.panel, label=self._fname)
        font = self.GetFont()
        font.SetWeight(wx.FONTWEIGHT_BOLD)
        if wx.Platform == '__WXMSW__':
            font.SetPointSize(12)
        else:
            font.SetPointSize(13)
        lbl.SetFont(font)
        head.Add(lbl, 0, wx.ALIGN_LEFT)

        errlbl = wx.StaticText(self.panel, label=_("File Stat Failed"))
        if wx.Platform == '__WXMSW__':
            font.SetPointSize(10)
        else:
            font.SetPointSize(11)
        font.SetWeight(wx.FONTWEIGHT_LIGHT)
        errlbl.SetFont(font)
        head.Add((5, 5), 0)
        head.Add(errlbl, 0, wx.ALIGN_LEFT)
        top.AddMany([((5, 5),), (bmp, 0, wx.ALIGN_LEFT), ((12, 12),), 
                     (head, 0, wx.ALIGN_LEFT), ((5, 5),)])

        # Central Area
        csizer = wx.BoxSizer(wx.VERTICAL)
        errbmp = wx.ArtProvider.GetBitmap(wx.ART_ERROR, wx.ART_CMN_DIALOG)
        errbmp = wx.StaticBitmap(self.panel, bitmap=errbmp)
        errmsg = wx.StaticText(self.panel, label=msg)
        errmsg.SetFont(font)
        errmsg.Wrap(225)
        errsz = wx.BoxSizer(wx.HORIZONTAL)
        errsz.AddMany([((8, 8)), (errmsg, 0, wx.ALIGN_LEFT), ((8, 8))])
        csizer.AddMany([((10, 10)), (top, 1, wx.EXPAND), ((10, 10)),
                        (wx.StaticLine(self.panel, style=wx.LI_HORIZONTAL), 0, wx.EXPAND),
                        ((20, 20)), (errbmp, 0, wx.ALIGN_CENTER),
                        ((10, 10)), (errsz, 0, wx.ALIGN_CENTER), ((10, 10)),
                        (wx.StaticLine(self.panel, style=wx.LI_HORIZONTAL), 0, wx.EXPAND),
                        ((10, 10))])
        self.panel.SetSizer(csizer)

    def __DoLayout(self):
        """Layout the dialog"""
        # Top Info
        top = wx.BoxSizer(wx.HORIZONTAL)
        head = wx.BoxSizer(wx.HORIZONTAL)
        lbl = wx.StaticText(self.panel, label=self._fname)
        fszlbl = wx.StaticText(self.panel, label=self._fstat['size'])
        font = self.GetFont()
        font.SetWeight(wx.FONTWEIGHT_BOLD)
        if wx.Platform == '__WXMSW__':
            font.SetPointSize(12)
        else:
            font.SetPointSize(13)
        lbl.SetFont(font)
        fszlbl.SetFont(font)
        head.Add(lbl, 0, wx.ALIGN_LEFT)
        head.AddStretchSpacer(2)
        head.Add(fszlbl, 1, wx.ALIGN_RIGHT)

        modlbl = wx.StaticText(self.panel, label="%s:  %s" % (_("Modified"),
                                                        self._fstat['mtime']))
        if wx.Platform == '__WXMSW__':
            font.SetPointSize(10)
        else:
            font.SetPointSize(11)

        font.SetWeight(wx.FONTWEIGHT_LIGHT)
        modlbl.SetFont(font)
        lblsize = wx.BoxSizer(wx.VERTICAL)
        lblsize.AddMany([(head, 1, wx.ALIGN_LEFT), ((3, 3),), 
                         (modlbl, 0, wx.ALIGN_LEFT | wx.ALIGN_BOTTOM)])

        top.AddMany([((5, 5)),
                     (self._bmp, 0, wx.ALIGN_LEFT),
                     ((12, 12)), (lblsize, 0, wx.ALIGN_LEFT), ((5, 5))])

        # Central Info
        center = wx.FlexGridSizer(6, 2, 3, 5)
        tlbl = wx.StaticText(self.panel, label=_("Kind") + ":")

        if self._ftype is None:
            self._ftxt.SetLabel(GetFileType(self._file))
        else:
            self._ftxt.SetLabel(self._ftype)

        szlbl = wx.StaticText(self.panel, label=_("Size") + ":")
        szval = wx.StaticText(self.panel, label=self._fstat['size'])
        loclbl = wx.StaticText(self.panel, label=_("Where") + ":")
        locval = wx.StaticText(self.panel, label=self._FormatLabel(self._file))
        ctime = wx.StaticText(self.panel, label=_("Created") + ":")
        cval = wx.StaticText(self.panel, label=self._fstat['ctime'])
        mtime = wx.StaticText(self.panel, label=_("Modified") + ":")
        mval = wx.StaticText(self.panel, label=self._fstat['mtime'])
        perm = wx.StaticText(self.panel, label=_("Permissions") + ":")
        pval = wx.StaticText(self.panel, label=self._fstat['perm'])
        for lbl in (tlbl, self._ftxt, szlbl, szval, loclbl, 
                    locval, ctime, cval, mtime, mval, perm, pval):
            lbl.SetFont(font)
            lbl.Wrap(200)
        center.AddMany([(tlbl, 0, wx.ALIGN_RIGHT), (self._ftxt, 0, wx.ALIGN_LEFT),
                        (szlbl, 0, wx.ALIGN_RIGHT), (szval, 0, wx.ALIGN_LEFT),
                        (loclbl, 0, wx.ALIGN_RIGHT), (locval, 0, wx.ALIGN_LEFT),
                        (ctime, 0, wx.ALIGN_RIGHT), (cval, 0, wx.ALIGN_LEFT),
                        (mtime, 0, wx.ALIGN_RIGHT), (mval, 0, wx.ALIGN_LEFT),
                        (perm, 0, wx.ALIGN_RIGHT), (pval, 0, wx.ALIGN_LEFT)])
        cmain = wx.BoxSizer(wx.HORIZONTAL)
        cmain.AddMany([((8, 8),), (center, 0, wx.ALIGN_CENTER), ((8, 8),)])

        # Main Layout
        msizer = wx.BoxSizer(wx.VERTICAL)
        msizer.AddMany([((10, 10)), (top, 0, wx.ALIGN_CENTER), ((10, 10),),
                        (wx.StaticLine(self.panel, style=wx.LI_HORIZONTAL), 1,
                                                    wx.EXPAND|wx.ALIGN_CENTER),
                        ((10, 10),), (cmain, 0, wx.ALIGN_TOP|wx.ALIGN_CENTER),
                        ((10, 10),),
                        (wx.StaticLine(self.panel, style=wx.LI_HORIZONTAL), 1,
                                                    wx.EXPAND|wx.ALIGN_CENTER),
                        ((10, 10),),
                        ])
        self.panel.SetSizer(msizer)

    def _FormatLabel(self, lbl):
        """Format the label to a suitable width wrapping as necessary"""
        lbl_len = len(lbl)
        part = self.GetTextExtent(lbl)[0] / 200
        if part > 1:
            split = lbl_len / part
            pieces = list()
            for chunk in xrange(part):
                if chunk == part - 1:
                    pieces.append(lbl[chunk * split:])
                else:
                    pieces.append(lbl[chunk * split:(chunk * split + split)])
            return os.linesep.join(pieces)
        return lbl

    def OnClose(self, evt):
        """Destroy ourselves on closer"""
        self.Destroy()
        evt.Skip()

    def SetBitmap(self, bmp):
        """Set the dialog bitmap
        @param bmp: wxBitmap

        """
        self._bmp.SetBitmap(bmp)
        self._bmp.Refresh()
        self.panel.Layout()

    def SetFileTypeLabel(self, lbl):
        """Set the file type label
        @param lbl: string

        """
        self._ftype = lbl
        self._ftxt.SetLabel(lbl)
        self.panel.Layout()

#-----------------------------------------------------------------------------#
# Utility Functions

def CalcSize(bits):
    """Calculate the best display version of the size of a given file
    1024 = 1KB, 1024KB = 1MB, ...
    @param bits: size of file returned by stat
    @return: formatted string representation of value

    """
    val = ('bytes', 'KB', 'MB', 'GB', 'TB')
    ind = 0
    while bits > 1024:
        bits = float(bits) / 1024.0
        ind += 1

    rval = "%.2f" % bits
    rval = rval.rstrip('.0')
    if not rval:
        rval = '0'
    rval = "%s %s" % (rval, val[min(ind, 4)])
    return rval

def GetFileType(fname):
    """Get what the type of the file is
    @param fname: file path

    """
    if os.path.isdir(fname):
        return _("Folder")

    mtype = mimetypes.guess_type(fname)[0]
    if mtype is not None:
        return mtype
    else:
        return _("Unknown")

