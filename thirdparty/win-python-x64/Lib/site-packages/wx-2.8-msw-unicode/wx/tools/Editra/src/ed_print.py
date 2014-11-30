###############################################################################
# Name: ed_print.py                                                           #
# Purpose: Editra's printer class                                             #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
Printer class for creating and managing printouts from a StyledTextCtrl.

Classes:
  - L{EdPrinter}: Class for managing printing and providing print dialogs
  - L{EdPrintout}: Scales and renders the given document to a printer.

@summary: Printer Classes for printing text from an STC

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__cvsid__ = "$Id: ed_print.py 67499 2011-04-15 20:33:40Z CJP $"
__revision__ = "$Revision: 67499 $"

#--------------------------------------------------------------------------#
# Imports
import wx
import wx.stc

# Editra Imports
import ed_glob
import util
import extern.stcprint as stcprint

_ = wx.GetTranslation
#--------------------------------------------------------------------------#

# Globals
COLOURMODES = { ed_glob.PRINT_BLACK_WHITE : wx.stc.STC_PRINT_BLACKONWHITE,
                ed_glob.PRINT_COLOR_WHITE : wx.stc.STC_PRINT_COLOURONWHITE,
                ed_glob.PRINT_COLOR_DEF   : wx.stc.STC_PRINT_COLOURONWHITEDEFAULTBG,
                ed_glob.PRINT_INVERT      : wx.stc.STC_PRINT_INVERTLIGHT,
                ed_glob.PRINT_NORMAL      : wx.stc.STC_PRINT_NORMAL }

#--------------------------------------------------------------------------#
class EdPrinter(object):
    """Printer Class for the editor
    @note: current font size is fixed at 12 point for printing

    """
    def __init__(self, parent, mode=ed_glob.PRINT_NORMAL):
        """Initializes the Printer
        @param parent: parent window
        @keyword mode: printer mode

        """
        super(EdPrinter, self).__init__()

        # Attributes
        self.stc = None
        self.title = wx.EmptyString
        self.parent = parent
        self.print_mode = mode
        self.print_data = wx.PrintData()
        self.margins = (wx.Point(15,15), wx.Point(15,15))

    def CreatePrintout(self):
        """Creates a printout of the current stc window
        @return: a printout object

        """
        colour = COLOURMODES[self.print_mode]
        dlg_data = wx.PageSetupDialogData(self.print_data)
        dlg_data.SetPrintData(self.print_data)
        dlg_data.SetMarginTopLeft(self.margins[0])
        dlg_data.SetMarginBottomRight(self.margins[1])
        fname = self.stc.GetFileName()
        printout = stcprint.STCPrintout(self.stc, page_setup_data=dlg_data, 
                                        print_mode=colour, title=self.title,
                                        job_title=fname)
        return printout

    def PageSetup(self):
        """Opens a print setup dialog and save print settings.
        @return: None

        """
        dlg_data = wx.PageSetupDialogData(self.print_data)
        dlg_data.SetPrintData(self.print_data)
    
        dlg_data.SetDefaultMinMargins(True)
        dlg_data.SetMarginTopLeft(self.margins[0])
        dlg_data.SetMarginBottomRight(self.margins[1])

        print_dlg = wx.PageSetupDialog(self.parent, dlg_data)
        if print_dlg.ShowModal() == wx.ID_OK:
            self.print_data = wx.PrintData(dlg_data.GetPrintData())
            self.print_data.SetPaperId(dlg_data.GetPaperId())
            self.margins = (dlg_data.GetMarginTopLeft(),
                            dlg_data.GetMarginBottomRight())
        print_dlg.Destroy()

    def Preview(self):
        """Preview the Print
        @return: None

        """
        printout = self.CreatePrintout()
        printout2 = self.CreatePrintout()
        preview = wx.PrintPreview(printout, printout2, self.print_data)
        preview.SetZoom(150)
        if preview.IsOk():
            pre_frame = wx.PreviewFrame(preview, self.parent,
                                             _("Print Preview"))
            dsize = wx.GetDisplaySize()
            pre_frame.SetInitialSize((self.stc.GetSize()[0],
                                          dsize.GetHeight() - 100))
            pre_frame.Initialize()
            pre_frame.Show()
        else:
            wx.MessageBox(_("Failed to create print preview"),
                          _("Print Error"),
                          style=wx.ICON_ERROR|wx.OK)

    def Print(self):
        """Prints the document
        @postcondition: the current document is printed

        """
        pdd = wx.PrintDialogData(self.print_data)
        printer = wx.Printer(pdd)
        printout = self.CreatePrintout()
        result = printer.Print(self.parent, printout)
        if result:
            dlg_data = printer.GetPrintDialogData()
            self.print_data = wx.PrintData(dlg_data.GetPrintData())
        elif printer.GetLastError() == wx.PRINTER_ERROR:
            wx.MessageBox(_("There was an error when printing.\n"
                            "Check that your printer is properly connected."),
                          _("Printer Error"),
                          style=wx.ICON_ERROR|wx.OK)
        printout.Destroy()

    def SetColourMode(self, mode):
        """Sets the color mode that the text is to be rendered with
        @param mode: mode to set the printer to use
        @return: whether mode was set or not
        @rtype: boolean

        """
        if mode in COLOURMODES:
            self.print_mode = mode
            ret = True
        else:
            ret = False
        return ret

    def SetStc(self, stc):
        """Set the stc we are printing for
        @param stc: instance of wx.stc.StyledTextCtrl
        @note: MUST be called prior to any other print operations

        """
        self.stc = stc
