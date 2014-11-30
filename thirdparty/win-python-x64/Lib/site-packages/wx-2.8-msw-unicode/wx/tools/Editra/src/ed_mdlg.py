###############################################################################
# Name: ed_mdlg.py                                                            #
# Purpose: Commonly used message dialogs                                      #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
This module provides a number of message dialogs that are commonly used
throughout Editra. Its purpose is to promote reuse of the common dialogs for
consistancy and reduction in redundant code.

@summary: Common dialogs and related convenience functions

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: ed_mdlg.py 66817 2011-01-29 21:32:20Z CJP $"
__revision__ = "$Revision: 66817 $"

#--------------------------------------------------------------------------#
# Imports
import wx
import wx.stc
from extern.embeddedimage import PyEmbeddedImage

# Editra Library
import ed_glob
import util
import eclib

#--------------------------------------------------------------------------#
# Globals

_ = wx.GetTranslation

FileIcon = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAAABHNCSVQICAgIfAhkiAAABthJ"
    "REFUWIW9l21sVFkZgJ/7Pd8zLdOWQloiIA2WbiqBLWhhdw3IAhuyRqMxq64uv1B/GMJqYvzK"
    "qqkmfqx/3UiC7q4QN5o1UnbRPyVLgdYtCAjdju2Uwhbabmd6O9PpdO7H8Ud7h2lnaLv+8E1u"
    "zj3vnXPe57znfd9zRqJcIseOHXsxGo1W5fP5nOu6xQ+OIwAXT+e6LrZtUygUdE3T3O7u7pP9"
    "/f03K8y5egkGgy2maTpiBXEcR1iWJXK5nBgbGxPJZFJ0d3eL3bt3H/4w9qSlinA43NbT03O5"
    "oaGBdDoNgBAC13WLreM4xb4QAtu2SaVSzMzMEI/HOX78+KGLFy+eWw2AXEYkSWKhLdWhKAqy"
    "LCPLMoqiFPuSJKFpGrFYjImJCUzTpLOzs7O1tfXg/wTgGaz0eIZVVS2+eyC6rhOLxRgaGiIS"
    "idDV1dXZ0tKyIkQZgGVZopJxb/XeqksBvCcQCCCEIJPJEIlEuHLlSueOHTuWhajogVJjnkG3"
    "JA48AE3Tit7wtsHv99Pf38/g4CCWZXH27NnOtra2R0KoSxXV1dWKruvFrXDd+bRzhUC47iJv"
    "2LZDKGigqgqTqSy1tbVks1nOnDnDyMgIwWCQXbt20dHR0XnixImn+/r63l5qrywLDh48uP/0"
    "6dPnFUXBNE0cy0Lz+9Grq3EdB5jPfyc7Q33cRyo1zcR4hnhjPXmhUJjLk0gkGLpxA3t2ltm5"
    "ORobGzEMI3306NHHUqnUvWU9sHXr1ng4HMY0TRRFwRcIYOdymAMDyJqGNZmm1nCItGzm0nWT"
    "F37Yx8c32Jz8js34/TkwdOK2Q9W2baiBAIV8nkwmQ01NTVVTU9OnL126dHJZgLVr18a91DIM"
    "A7/fz8TwMOaXn6chGmBNewsDH32Cb/xxlOvvJWleU8vVbD2dL/+Zw9fOM2FaCOHi/OznRJub"
    "sSYmiMViyLJMLpfzrxgDmqb5AAzDQFEUAHyFAi2BAsqhw5xSW3n5wizbxACnmsdpbdV4IxNk"
    "w2QM4wOTUP8gbjhM1tBxFgqVYRgEAgE0TVtqrhzAsqwcgKIoxYj3r1vLXz73I875d3H15k1+"
    "teMuTwUNHiR0JmerOLAlTu+4Rr69HXfGxhEOuqZh6Dr5hSzy+/0YhlEWc2UAyWTyfXhYjKYn"
    "U3z/lb9zJRVAQqLev4XaDQ5EFLJOlM0HdnI7rfLcrx/Q9ewetoyNku4fJuTzEfL7wedDCIGq"
    "qchyedaXabq7uycymUyxPxeuYn+Dj4vSGxwI/pO3bmn8picMbU1sfuEQd2b8dLzyHx70K7yU"
    "qIP9e1nf+jFq6msxAJ/Ph67rqIpK6cn6SIBkMlnI5/MAFCyLGl2ifUcz6X/0ccT3Lvvb5kik"
    "6/nbhTR/Opei7bnXyZq3ee17Phx5kluBOq637OHUhQQaYPh8xYIFiBW3AJA8V3kb5kQi3Pv8"
    "19i+r4Uv3XufjrONvPhbhTX2X3n1x4+z75Nb4NYgz1h3MXqv8qrSzC97E3zxQDPBUDXZhQJW"
    "Sco8oKqqJMnzP/ZAFKDRdWBgki80zrK+apzEgxDPf7aVffubYFzCHpki2NWLoZnkwptI3A0x"
    "en9s0TyVYqDMA7ZtC89RHrWwHXJ3htHyc4RrdL7ZrnAnHeP1y2v5RPRdmqU8qgY8+yl+/2+D"
    "H/TYfGWPReO6mkXzrMoDpeIFjSRc3A8mcadSzF4e4EhdhiNtGW6PxXjtXzroM1ybinKgt56X"
    "+mf5ae0Ffnd8O1owTi6XWxagUgwgxOJYEbYNd+8iWRZzcwX87wi++pEC4ztruJbaxTPnrzI2"
    "PcxeaZQ3Iwl8l3sxx48SqlvsyVUBWJZVBChts/k8SiaDpRuEJoM0PxnDvHqf0fvDtFfd5CfG"
    "NVpHhsjcGGFQ1YjrKhEe1hOgWFlX9IAnkiThAqFNm1j/1jkkSSJSFeK9xCjf+sXbhKI+/vDt"
    "x2nZ+BnE0JOkbBc34KdOUQisW4dtO4sAVuWBpeLaNqphEN24sagbJc2e9ga++/XDoEQQgPtY"
    "I1EPHLALBWyrgFR+4q8M4BF7rXcT9t73bt/EUzu3AGDbNm5Jnns3ZSHmxwtAkh4d66sCmL+O"
    "C2D+WlawCsj24vshzOe5Bzs/VEIIgbxQV7xFfGiA+VYsTCYX/x94xh+CLh7vSaUCVPz2yC9L"
    "JvBWWwq5VCfLi2/SlWCWSpkHVFWVFg6ORYMrXSaWg60kmqatfB+wbduZmpoiHA4zPT1d1Jf+"
    "PxBCIFyBK9zyolXS9941TSMUClEoFMrO40r+qQ6FQk/Islznuq5NyREaCARkwzBk27ZFPp93"
    "LcsqO14fIaokSblMJvMOkFzlmP+P/BeZah5l10evBAAAAABJRU5ErkJggg==")

#--------------------------------------------------------------------------#

def OpenErrorDlg(parent, fname, err):
    """Show a file open error dialog
    @param parent: parent window
    @param fname: file that failed to open
    @param err: error message

    """
    argmap = dict(filename=fname, errormsg=err)
    dlg = wx.MessageDialog(parent,
                           _("Editra could not open %(filename)s\n\n"
                             "Error:\n%(errormsg)s") % \
                           argmap, _("Error Opening File"),
                           style=wx.OK|wx.CENTER|wx.ICON_ERROR)
    dlg.CenterOnParent()
    result = dlg.ShowModal()
    dlg.Destroy()
    return result

def SaveErrorDlg(parent, fname, err):
    """Show a file save error modal dialog
    @param parent: window that the dialog is the child of
    @param fname: name of file that error occured
    @param err: the err message/description
    @return: wxID_OK if dialog was shown and dismissed properly

    """
    argmap = dict(filename=fname, errormsg=err)
    dlg = wx.MessageDialog(parent,
                           _("Failed to save file: %(filename)s\n\n"
                             "Error:\n%(errormsg)s") % argmap,
                           _("Save Error"), wx.OK|wx.ICON_ERROR)
    dlg.CenterOnParent()
    result = dlg.ShowModal()
    dlg.Destroy()
    return result

#--------------------------------------------------------------------------#

class EdFileInfoDlg(eclib.FileInfoDlg):
    """File information dialog"""
    def __init__(self, parent, fname):
        """General file information dialog
        @param parent: parent window
        @param fname: file path

        """
        super(EdFileInfoDlg, self).__init__(parent, fname=fname, ftype=None, 
                                            bmp=FileIcon.GetBitmap())

        # Setup
        self.SetFileTypeLabel(util.GetFileType(fname))

#--------------------------------------------------------------------------#

class EdFormatEOLDlg(eclib.ChoiceDialog):
    """Dialog for selecting EOL format"""
    def __init__(self, parent, msg=u'', title=u'', selection=0):
        """Create the dialog
        @keyword selection: default selection (wx.stc.STC_EOL_*)

        """
        choices = [_("Old Machintosh (\\r)"), _("Unix (\\n)"),
                   _("Windows (\\r\\n)")]
        self._eol = [wx.stc.STC_EOL_CR, wx.stc.STC_EOL_LF, wx.stc.STC_EOL_CRLF]
        idx = self._eol.index(selection)
        super(EdFormatEOLDlg, self).__init__(parent, msg=msg, title=title,
                                             choices=choices,
                                             style=wx.YES_NO|wx.YES_DEFAULT)
        self.SetSelection(idx)

        # Setup
        bmp = wx.ArtProvider.GetBitmap(str(ed_glob.ID_DOCPROP), wx.ART_OTHER)
        if bmp.IsOk():
            self.SetBitmap(bmp)
        self.CenterOnParent()

    def GetSelection(self):
        """Get the selected eol mode
        @return: wx.stc.STC_EOL_*

        """
        sel = super(EdFormatEOLDlg, self).GetSelection()
        return self._eol[sel]
