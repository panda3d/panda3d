#----------------------------------------------------------------------
# Name:        wx.lib.pdfwin
# Purpose:     A class that allows the use of the Acrobat PDF reader
#              ActiveX control
#
# Author:      Robin Dunn
#
# Created:     22-March-2004
# RCS-ID:      $Id: pdfwin.py 64237 2010-05-06 18:25:24Z RD $
# Copyright:   (c) 2008 by Total Control Software
# Licence:     wxWindows license
#----------------------------------------------------------------------

import wx

_min_adobe_version = None

def get_min_adobe_version():
    return _min_adobe_version

def get_acroversion():
    " Included for backward compatibility"
    return _min_adobe_version

#----------------------------------------------------------------------
            
if  wx.PlatformInfo[1] == 'wxMSW':
    import wx.lib.activex
    import comtypes.client as cc
    import comtypes
    import ctypes

    try:            # Adobe Reader >= 7.0
        cc.GetModule( ('{05BFD3F1-6319-4F30-B752-C7A22889BCC4}', 1, 0) )
        progID = 'AcroPDF.PDF.1'
        _min_adobe_version = 7.0
    except:
        try:        # Adobe Reader 5 or 6
            cc.GetModule( ('{CA8A9783-280D-11CF-A24D-444553540000}', 1, 0) )
            progID = 'PDF.PdfCtrl.5'
            _min_adobe_version = 5.0
        except:
            pass    # Adobe Reader not installed (progID is not defined)
                    # Use get_min_adobe_version() before instantiating PDFWindow 

    #------------------------------------------------------------------------------

    class PDFWindow(wx.lib.activex.ActiveXCtrl):
        def __init__(self, parent, id=-1, pos=wx.DefaultPosition,
                     size=wx.DefaultSize, style=0, name='PDFWindow'):
            wx.lib.activex.ActiveXCtrl.__init__(self, parent, progID,
                                                    id, pos, size, style, name)
            self.Bind(wx.EVT_WINDOW_DESTROY, self.OnDestroyWindow)

        def OnDestroyWindow(self, event):
            wx.CallAfter(self.FreeDlls)

        def FreeDlls(self):    
            """
            Unloads any DLLs that are no longer in use when all COM object instances are
            released. This prevents the error 'The instruction at "0x0700609c" referenced
            memory at "0x00000014". The memory could not be read' when application closes
            """
            ctypes.windll.ole32.CoFreeUnusedLibraries()

        def LoadFile(self, fileName):
            """
            Opens and displays the specified document within the browser.
            """
            return self.ctrl.LoadFile(fileName)

        def GetVersions(self):
            """
            Deprecated: No longer available - do not use.
            """
            return self.ctrl.GetVersions()

        def Print(self):
            """
            Prints the document according to the specified options in a user dialog box.
            """
            return self.ctrl.Print()

        def goBackwardStack(self):
            """
            Goes to the previous view on the view stack, if it exists.
            """
            return self.ctrl.goBackwardStack()
            
        def goForwardStack(self):
            """
            Goes to the next view on the view stack, if it exists.
            """
            return self.ctrl.goForwardStack()
            
        def gotoFirstPage(self):
            """
            Goes to the first page in the document.
            """
            return self.ctrl.gotoFirstPage()
            
        def gotoLastPage(self):
            """
            Goes to the last page in the document.
            """
            return self.ctrl.gotoLastPage()
            
        def gotoNextPage(self):
            """
            Goes to the next page in the document, if it exists
            """
            return self.ctrl.gotoNextPage()
            
        def gotoPreviousPage(self):
            """
            Goes to the previous page in the document, if it exists.
            """
            return self.ctrl.gotoPreviousPage()
            
        def printAll(self):
            """
            Prints the entire document without displaying a user
            dialog box.  The current printer, page settings, and job
            settings are used.  This method returns immediately, even
            if the printing has not completed.
            """
            return self.ctrl.printAll()
            
        def printAllFit(self, shrinkToFit):
            """
            Prints the entire document without a user dialog box, and
            (if shrinkToFit) shrinks pages as needed to fit the
            imageable area of a page in the printer.
            """
            return self.ctrl.printAllFit(shrinkToFit)
            
        def printPages(self, from_, to):
            """
            Prints the specified pages without displaying a user dialog box.
            """
            return self.ctrl.printPages(from_, to)
            
        def printPagesFit(self, from_, to, shrinkToFit):
            """
            Prints the specified pages without displaying a user
            dialog box, and (if shrinkToFit) shrinks pages as needed
            to fit the imageable area of a page in the printer.
            """
            return self.ctrl.printPagesFit( from_, to, shrinkToFit)
            
        def printWithDialog(self):
            """
            Prints the document according to the specified options in
            a user dialog box. These options may include embedded
            printing and specifying which printer is to be used.

            NB. The page range in the dialog defaults to
            'From Page 1 to 1' - Use Print() above instead. (dfh) 
            """
            return self.ctrl.printWithDialog()
            
        def setCurrentHighlight(self, a, b, c, d):
            return self.ctrl.setCurrentHighlight(a, b, c, d)
            
        def setCurrentPage(self, npage):
            """
            Goes to the specified page in the document.  Maintains the
            current location within the page and zoom level.  npage is
            the page number of the destination page.  The first page
            in a document is page 0.

            ## Oh no it isn't! The first page is 1 (dfh)
            """
            return self.ctrl.setCurrentPage(npage)
            
        def setLayoutMode(self, layoutMode):
            """
            LayoutMode possible values:

                =================  ====================================
                'DontCare'         use the current user preference
                'SinglePage'       use single page mode (as in pre-Acrobat
                    3.0 viewers)
                'OneColumn'        use one-column continuous mode
                'TwoColumnLeft'    use two-column continuous mode, first
                    page on the left
                'TwoColumnRight'   use two-column continuous mode, first
                    page on the right
                =================  ====================================
            """
            return self.ctrl.setLayoutMode(layoutMode)
            
        def setNamedDest(self, namedDest):
            """
            Changes the page view to the named destination in the specified string.
            """
            return self.ctrl.setNamedDest(namedDest)
            
        def setPageMode(self, pageMode):
            """
            Sets the page mode to display the document only, or to
            additionally display bookmarks or thumbnails.  pageMode =
            'none' or 'bookmarks' or 'thumbs'.

            ## NB.'thumbs' is case-sensitive, the other are not (dfh)
            """   
            return self.ctrl.setPageMode(pageMode)
            
        def setShowScrollbars(self, On):
            """
            Determines whether scrollbars will appear in the document
            view.

            ## NB. If scrollbars are off, the navigation tools disappear as well (dfh)
            """
            return self.ctrl.setShowScrollbars(On)
            
        def setShowToolbar(self, On):
            """
            Determines whether a toolbar will appear in the application.
            """
            return self.ctrl.setShowToolbar(On)
            
        def setView(self, viewMode):
            """
            Determines how the page will fit in the current view.
            viewMode possible values:

                ========  ==============================================
                'Fit'     fits whole page within the window both vertically
                    and horizontally.
                'FitH'    fits the width of the page within the window.
                'FitV'    fits the height of the page within the window.
                'FitB'    fits bounding box within the window both vertically
                    and horizontally.
                'FitBH'   fits the width of the bounding box within the window.
                'FitBV'   fits the height of the bounding box within the window.
                ========  ==============================================
            """
            return self.ctrl.setView(viewMode)
            
        def setViewRect(self, left, top, width, height):
            """
            Sets the view rectangle according to the specified coordinates.

            :param left:   The upper left horizontal coordinate.
            :param top:    The vertical coordinate in the upper left corner.
            :param width:  The horizontal width of the rectangle.
            :param height: The vertical height of the rectangle.
            """
            return self.ctrl.setViewRect(left, top, width, height)
            
        def setViewScroll(self, viewMode, offset):
            """
            Sets the view of a page according to the specified string.
            Depending on the view mode, the page is either scrolled to
            the right or scrolled down by the amount specified in
            offset. Possible values of viewMode are as in setView
            above. offset is the horizontal or vertical coordinate
            positioned either at the left or top edge.
            """    
            return self.ctrl.setViewScroll(viewMode, offset)
            
        def setZoom(self, percent):
            """
            Sets the magnification according to the specified value
            expressed as a percentage (float)
            """
            return self.ctrl.setZoom(percent)
            
        def setZoomScroll(self, percent, left, top):
            """
            Sets the magnification according to the specified value,
            and scrolls the page view both horizontally and vertically
            according to the specified amounts.

            :param left:  the horizontal coordinate positioned at the left edge.
            :param top:   the vertical coordinate positioned at the top edge.
            """
            return self.ctrl.setZoomScroll(percent, left, top)


#------------------------------------------------------------------------------




if __name__ == '__main__':
    app = wx.App(False)
    frm = wx.Frame(None, title="AX Test Window")
    
    pdf = PDFWindow(frm)
    
    frm.Show()
    import wx.lib.inspection
    wx.lib.inspection.InspectionTool().Show()
    app.MainLoop()
                                 



