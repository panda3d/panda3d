#-----------------------------------------------------------------------------
# Name:        stcprint.py
# Purpose:     wx.StyledTextCtrl printing support
#
# Author:      Rob McMullen
#
# Created:     2009
# RCS-ID:      $Id: stcprint.py 67499 2011-04-15 20:33:40Z CJP $
# Copyright:   (c) 2009 Rob McMullen <robm@users.sourceforge.net>
#              (c) 2007 Cody Precord <staff@editra.org>
# License:     wxWidgets
#-----------------------------------------------------------------------------
"""Printing support for the wx.StyledTextCtrl

Concrete implementation of the wx.Printout class to generate a print preview
and paper copies of the contents of a wx.StyledTextCtrl.  This was written
for U{Peppy<http://peppy.flipturn.org>} but has been designed as a standalone
class with no dependencies on peppy.  It can be used for general purpose
printing or print preview of a wx.StyledTextCtrl.  See the demo application at
the end of this file for more information.

I used code from U{Editra<http://www.editra.org>} as a starting point; other
pointers came from the wxPython mailing list, and lots was just pure ol'
trial and error because I couldn't find much specific documentation on the
FormatRange method of the STC.

NOTE: there are issues with certain scale factors when using print preview on
MSW.  Some scale factors seem to work correctly, like 150%, but other smaller
scale factors cause the preview font size to fluctuate.  Some zoom levels will
use very small fonts and render all the lines in the top half of the page,
while other zoom levels use an incorrectly large font and render lines off the
bottom of the page.  The printed output is unaffected, however, and renders
the correct number of lines.
"""

import os

import wx
import wx.stc
_ = wx.GetTranslation

class STCPrintout(wx.Printout):
    """Specific printing support of the wx.StyledTextCtrl for the wxPython
    framework
    
    This class can be used for both printing to a printer and for print preview
    functions.  Unless otherwise specified, the print is scaled based on the
    size of the current font used in the STC so that specifying a larger font
    produces a larger font in the printed output (and correspondingly fewer
    lines per page).  Alternatively, you can eihdec specify the number of
    lines per page, or you can specify the print font size in points which
    produces a constant number of lines per inch regardless of the paper size.
    
    Note that line wrapping in the source STC is currently ignored and lines
    will be truncated at the right margin instead of wrapping.  The STC doesn't
    provide a convenient method for determining where line breaks occur within
    a wrapped line, so it may be a difficult task to ever implement printing
    with line wrapping using the wx.StyledTextCtrl.FormatRange method.
    """
    debuglevel = 0
    
    def __init__(self, stc, page_setup_data=None, print_mode=None, title=None, 
                 border=False, lines_per_page=None, output_point_size=None,
                 job_title=None):
        """Constructor.
        
        @param stc: wx.StyledTextCtrl to print
        
        @kwarg page_setup_data: optional wx.PageSetupDialogData instance that
        is used to determine the margins of the page.
        
        @kwarg print_mode: optional; of the wx.stc.STC_PRINT_*
        flags indicating how to render color text.  Defaults to
        wx.stc.STC_PRINT_COLOURONWHITEDEFAULTBG
        
        @kwarg title: optional text string to use as the title which will be
        centered above the first line of text on each page
        
        @kwarg border: optional flag indicating whether or not to draw a black
        border around the text on each page
        
        @kwarg lines_per_page: optional integer that will force the page to
        contain the specified number of lines.  Either of C{output_point_size}
        and C{lines_per_page} fully specifies the page, so if both are
        specified, C{lines_per_page} will be used.
        
        @kwarg output_point_size: optional integer that will force the output
        text to be drawn in the specified point size.  (Note that there are
        72 points per inch.) If not specified, the point size of the text in
        the STC will be used unless C{lines_per_page} is specified.  Either of
        C{output_point_size} and C{lines_per_page} fully specifies the page,
        so if both are specified, C{lines_per_page} will be used.
        """
        if not job_title:
            job_title = wx.PrintoutTitleStr
        wx.Printout.__init__(self, job_title)
        self.stc = stc
        if print_mode:
            self.print_mode = print_mode
        else:
            self.print_mode = wx.stc.STC_PRINT_COLOURONWHITEDEFAULTBG
        if title is not None:
            self.title = title
        else:
            self.title = ""
        if page_setup_data is None:
            self.top_left_margin = wx.Point(15,15)
            self.bottom_right_margin = wx.Point(15,15)
        else:
            self.top_left_margin = page_setup_data.GetMarginTopLeft()
            self.bottom_right_margin = page_setup_data.GetMarginBottomRight()
        
        try:
            value = float(output_point_size)
            if value > 0.0:
                self.output_point_size = value
        except (TypeError, ValueError):
            self.output_point_size = None
        
        try:
            value = int(lines_per_page)
            if value > 0:
                self.user_lines_per_page = value
        except (TypeError, ValueError):
            self.user_lines_per_page = None
        
        self.border_around_text = border
        
        self.setHeaderFont()
    
    def OnPreparePrinting(self):
        """Called once before a print job is started to set up any defaults.
        
        """
        dc = self.GetDC()
        self._calculateScale(dc)
        self._calculatePageCount()
    
    def _calculateScale(self, dc):
        """Scale the DC
        
        This routine scales the DC based on the font size, determines the
        number of lines on a page, and saves some useful pixel locations like
        the top left corner and the width and height of the drawing area in
        logical coordinates.
        """
        if self.debuglevel > 0:
            print
        
        dc.SetFont(self.stc.GetFont())
        
        # Calculate pixels per inch of the various devices.  The dc_ppi will be
        # equivalent to the page or screen PPI if the target is the printer or
        # a print preview, respectively.
        page_ppi_x, page_ppi_y = self.GetPPIPrinter()
        screen_ppi_x, screen_ppi_y = self.GetPPIScreen()
        dc_ppi_x, dc_ppi_y = dc.GetPPI()
        if self.debuglevel > 0:
            print("printer ppi: %dx%d" % (page_ppi_x, page_ppi_y))
            print("screen ppi: %dx%d" % (screen_ppi_x, screen_ppi_y))
            print("dc ppi: %dx%d" % (dc_ppi_x, dc_ppi_y))
        
        # Calculate paper size.  Note that this is the size in pixels of the
        # entire paper, which may be larger than the printable range of the
        # printer.  We need to use the entire paper size because we calculate
        # margins ourselves.  Note that GetPageSizePixels returns the
        # dimensions of the printable area.
        px, py, pw, ph = self.GetPaperRectPixels()
        page_width_inch = float(pw) / page_ppi_x
        page_height_inch = float(ph) / page_ppi_y
        if self.debuglevel > 0:
            print("page pixels: %dx%d" % (pw, ph))
            print("page size: %fx%f in" % (page_width_inch, page_height_inch))
        
        dw, dh = dc.GetSizeTuple()
        dc_pixels_per_inch_x = float(dw) / page_width_inch
        dc_pixels_per_inch_y = float(dh) / page_height_inch
        if self.debuglevel > 0:
            print("device pixels: %dx%d" % (dw, dh))
            print("device pixels per inch: %fx%f" % (dc_pixels_per_inch_x, dc_pixels_per_inch_y))
        
        # Calculate usable page size
        page_height_mm = page_height_inch * 25.4
        margin_mm = self.top_left_margin[1] + self.bottom_right_margin[1]
        usable_page_height_mm = page_height_mm - margin_mm
        
        # Lines per page is then the number of lines (based on the point size
        # reported by wx) that will fit into the usable page height
        self.lines_pp = self._calculateLinesPerPage(dc, usable_page_height_mm)
        
        # The final DC scale factor is then the ratio of the total height in
        # pixels inside the margins to the number of pixels that it takes to
        # represent the number of lines
        dc_margin_pixels = float(dc_pixels_per_inch_y) * margin_mm / 25.4
        dc_usable_pixels = dh - dc_margin_pixels
        page_to_dc = self._calculateScaleFactor(dc, dc_usable_pixels, self.lines_pp)

        dc.SetUserScale(page_to_dc, page_to_dc)

        if self.debuglevel > 0:
            print("Usable page height: %f in" % (usable_page_height_mm / 25.4))
            print("Usable page pixels: %d" % dc_usable_pixels)
            print("lines per page: %d" % self.lines_pp)
            print("page_to_dc: %f" % page_to_dc)

        self.x1 = dc.DeviceToLogicalXRel(float(self.top_left_margin[0]) / 25.4 * dc_pixels_per_inch_x)
        self.y1 = dc.DeviceToLogicalXRel(float(self.top_left_margin[1]) / 25.4 * dc_pixels_per_inch_y)
        self.x2 = dc.DeviceToLogicalXRel(dw) - dc.DeviceToLogicalXRel(float(self.bottom_right_margin[0]) / 25.4 * dc_pixels_per_inch_x)
        self.y2 = dc.DeviceToLogicalYRel(dh) - dc.DeviceToLogicalXRel(float(self.bottom_right_margin[1]) / 25.4 * dc_pixels_per_inch_y)
        page_height = self.y2 - self.y1

        #self.lines_pp = int(page_height / dc_pixels_per_line)
        
        if self.debuglevel > 0:
            print("page size: %d,%d -> %d,%d, height=%d" % (int(self.x1), int(self.y1), int(self.x2), int(self.y2), page_height))
    
    def _calculateLinesPerPage(self, dc, usable_page_height_mm):
        """Calculate the number of lines that will fit on the page.
        
        @param dc: the Device Context
        
        @param usable_page_height_mm: height in mm of the printable part of the
        page (i.e.  with the border height removed)
        
        @returns: the number of lines on the page
        """
        if self.user_lines_per_page is not None:
            return self.user_lines_per_page
        
        font = dc.GetFont()
        if self.output_point_size is not None:
            points_per_line = self.output_point_size
        else:
            points_per_line = font.GetPointSize()
        
        # desired lines per mm based on point size.  Note: printer points are
        # defined as 72 points per inch
        lines_per_inch = 72.0 / float(points_per_line)
        
        if self.debuglevel > 0:
            print("font: point size per line=%d" % points_per_line)
            print("font: lines per inch=%f" % lines_per_inch)
            
        # Lines per page is then the number of lines (based on the point size
        # reported by wx) that will fit into the usable page height
        return float(usable_page_height_mm) / 25.4 * lines_per_inch

    def _calculateScaleFactor(self, dc, dc_usable_pixels, lines_pp):
        """Calculate the scale factor for the DC to fit the number of lines
        onto the printable area
        
        @param dc: the Device Context
        
        @param dc_usable_pixels: the number of pixels that defines usable
        height of the printable area
        
        @param lines_pp: the number of lines to fit into the printable area
        
        @returns: the scale facter to be used in wx.DC.SetUserScale
        """
        # actual line height in pixels according to the DC
        dc_pixels_per_line = dc.GetCharHeight()
        
        # actual line height in pixels according to the STC.  This can be
        # different from dc_pixels_per_line even though it is the same font.
        # Don't know why this is the case; maybe because the STC takes into
        # account additional spacing?
        stc_pixels_per_line = self.stc.TextHeight(0)
        if self.debuglevel > 0:
            print("font: dc pixels per line=%d" % dc_pixels_per_line)
            print("font: stc pixels per line=%d" % stc_pixels_per_line)
        
        # Platform dependency alert: I don't know why this works, but through
        # experimentation it seems like the scaling factor depends on
        # different font heights depending on the platform.
        if wx.Platform == "__WXMSW__":
            # On windows, the important font height seems to be the number of
            # pixels reported by the STC
            page_to_dc = float(dc_usable_pixels) / (stc_pixels_per_line * lines_pp)
        else:
            # Linux and Mac: the DC font height seems to be the correct height
            page_to_dc = float(dc_usable_pixels) / (dc_pixels_per_line * lines_pp)
        return page_to_dc

    def _calculatePageCount(self, attempt_wrap=False):
        """Calculates offsets into the STC for each page
        
        This pre-calculates the page offsets for each page to support print
        preview being able to seek backwards and forwards.
        """
        page_offsets = []
        page_line_start = 0
        lines_on_page = 0
        num_lines = self.stc.GetLineCount()
        
        line = 0
        while line < num_lines:
            if attempt_wrap:
                wrap_count = self.stc.WrapCount(line)
                if wrap_count > 1 and self.debuglevel > 0:
                    print("found wrapped line %d: %d" % (line, wrap_count))
            else:
                wrap_count = 1
            
            # If the next line pushes the count over the edge, mark a page and
            # start the next page
            if lines_on_page + wrap_count > self.lines_pp:
                start_pos = self.stc.PositionFromLine(page_line_start)
                end_pos = self.stc.GetLineEndPosition(page_line_start + lines_on_page - 1)
                if self.debuglevel > 0:
                    print("Page: line %d - %d" % (page_line_start, page_line_start + lines_on_page))
                page_offsets.append((start_pos, end_pos))
                page_line_start = line
                lines_on_page = 0
            lines_on_page += wrap_count
            line += 1
        
        if lines_on_page > 0:
            start_pos = self.stc.PositionFromLine(page_line_start)
            end_pos = self.stc.GetLineEndPosition(page_line_start + lines_on_page)
            page_offsets.append((start_pos, end_pos))
        
        self.page_count = len(page_offsets)
        self.page_offsets = page_offsets
        if self.debuglevel > 0:
            print("page offsets: %s" % self.page_offsets)

    def _getPositionsOfPage(self, page):
        """Get the starting and ending positions of a page
        
        @param page: page number
        
        @returns: tuple containing the start and end positions that can be
        passed to FormatRange to render a page
        """
        page -= 1
        start_pos, end_pos = self.page_offsets[page]
        return start_pos, end_pos

    def GetPageInfo(self):
        """Return the valid page ranges.
        
        Note that pages are numbered starting from one.
        """
        return (1, self.page_count, 1, self.page_count)

    def HasPage(self, page):
        """Returns True if the specified page is within the page range
        
        """
        return page <= self.page_count

    def OnPrintPage(self, page):
        """Draws the specified page to the DC

        @param page: page number to render
        """
        dc = self.GetDC()
        self._calculateScale(dc)

        self._drawPageContents(dc, page)
        self._drawPageHeader(dc, page)
        self._drawPageBorder(dc)

        return True
    
    def _drawPageContents(self, dc, page):
        """Render the STC window into a DC for printing.
        
        Force the right margin of the rendered window to be huge so the STC
        won't attempt word wrapping.
        
        @param dc: the device context representing the page
        
        @param page: page number
        """
        start_pos, end_pos = self._getPositionsOfPage(page)
        render_rect = wx.Rect(self.x1, self.y1, 32000, self.y2)
        page_rect = wx.Rect(self.x1, self.y1, self.x2, self.y2)

        self.stc.SetPrintColourMode(self.print_mode)
        edge_mode = self.stc.GetEdgeMode()
        self.stc.SetEdgeMode(wx.stc.STC_EDGE_NONE)
        end_point = self.stc.FormatRange(True, start_pos, end_pos, dc, dc,
                                        render_rect, page_rect)
        self.stc.SetEdgeMode(edge_mode)
    
    def _drawPageHeader(self, dc, page):
        """Draw the page header into the DC for printing
        
        @param dc: the device context representing the page
        
        @param page: page number
        """
        # Set font for title/page number rendering
        dc.SetFont(self.getHeaderFont())
        dc.SetTextForeground ("black")
        dum, yoffset = dc.GetTextExtent(".")
        yoffset /= 2
        if self.title:
            title_w, title_h = dc.GetTextExtent(self.title)
            dc.DrawText(self.title, self.x1, self.y1 - title_h - yoffset)

        # Page Number
        page_lbl = _("Page: %d") % page
        pg_lbl_w, pg_lbl_h = dc.GetTextExtent(page_lbl)
        dc.DrawText(page_lbl, self.x2 - pg_lbl_w, self.y1 - pg_lbl_h - yoffset)
    
    def setHeaderFont(self, point_size=10, family=wx.FONTFAMILY_SWISS,
                      style=wx.FONTSTYLE_NORMAL, weight=wx.FONTWEIGHT_NORMAL):
        """Set the font to be used as the header font
        
        @param point_size: point size of the font
        
        @param family: one of the wx.FONTFAMILY_* values, e.g.
        wx.FONTFAMILY_SWISS, wx.FONTFAMILY_ROMAN, etc.
        
        @param style: one of the wx.FONTSTYLE_* values, e.g.
        wxFONTSTYLE_NORMAL, wxFONTSTYLE_ITALIC, etc.
        
        @param weight: one of the wx.FONTWEIGHT_* values, e.g.
        wx.FONTWEIGHT_NORMAL, wx.FONTWEIGHT_LIGHT, etc.
        """
        self.header_font_point_size = point_size
        self.header_font_family = family
        self.header_font_style = style
        self.header_font_weight = weight
    
    def getHeaderFont(self):
        """Returns the font to be used to draw the page header text
        
        @returns: wx.Font instance
        """
        point_size = self.header_font_point_size
        font = wx.Font(point_size, self.header_font_family,
                       self.header_font_style, self.header_font_weight)
        return font

    def _drawPageBorder(self, dc):
        """Draw the page border into the DC for printing
        
        @param dc: the device context representing the page
        """
        if self.border_around_text:
            dc.SetPen(wx.BLACK_PEN)
            dc.SetBrush(wx.TRANSPARENT_BRUSH)
            dc.DrawRectangle(self.x1, self.y1, self.x2 - self.x1 + 1, self.y2 - self.y1 + 1)


if __name__ == "__main__":
    import sys
    import __builtin__
    __builtin__._ = unicode
    
    # Set up sample print data
    top_left_margin = wx.Point(15,15)
    bottom_right_margin = wx.Point(15,15)
    
    def wrap(text, width=80):
        """A word-wrap function that preserves existing line breaks
        and most spaces in the text.
        
        Expects that existing line breaks are posix newlines (\n).
        
        http://code.activestate.com/recipes/148061/
        """
        return reduce(lambda line, word, width=width: '%s%s%s' %
                      (line,
                       ' \n'[(len(line)-line.rfind('\n')-1
                             + len(word.split('\n',1)[0]
                                  ) >= width)],
                       word),
                      text.split(' ')
                     )

    class TestSTC(wx.stc.StyledTextCtrl):
        def __init__(self, *args, **kwargs):
            wx.stc.StyledTextCtrl.__init__(self, *args, **kwargs)
            self.SetMarginType(0, wx.stc.STC_MARGIN_NUMBER)
            self.SetMarginWidth(0, 32)

    class Frame(wx.Frame):
        def __init__(self, *args, **kwargs):
            super(self.__class__, self).__init__(*args, **kwargs)

            self.stc = TestSTC(self, -1)

            self.CreateStatusBar()
            menubar = wx.MenuBar()
            self.SetMenuBar(menubar)  # Adding the MenuBar to the Frame content.
            menu = wx.Menu()
            menubar.Append(menu, "File")
            self.menuAdd(menu, "Open", "Open File", self.OnOpenFile)
            menu.AppendSeparator()
            self.menuAdd(menu, "Print Preview", "Display print preview", self.OnPrintPreview)
            self.menuAdd(menu, "Print", "Print to printer or file", self.OnPrint)
            menu.AppendSeparator()
            self.menuAdd(menu, "Quit", "Exit the pragram", self.OnQuit)
            
            self.print_data = wx.PrintData()
            self.print_data.SetPaperId(wx.PAPER_LETTER)


        def loadFile(self, filename, word_wrap=False):
            fh = open(filename)
            text = fh.read()
            if word_wrap:
                text = wrap(text)
            self.stc.SetText(fh.read())
        
        def loadSample(self, paragraphs=10, word_wrap=False):
            lorem_ipsum = u"""\
Lorem ipsum dolor sit amet, consectetuer adipiscing elit.  Vivamus mattis
commodo sem.  Phasellus scelerisque tellus id lorem.  Nulla facilisi.
Suspendisse potenti.  Fusce velit odio, scelerisque vel, consequat nec,
dapibus sit amet, tortor.

Vivamus eu turpis.  Nam eget dolor.  Integer at elit.  Praesent mauris.  Nullam non nulla at nulla tincidunt malesuada. Phasellus id ante.  Sed mauris.  Integer volutpat nisi non diam.

Etiam elementum.  Pellentesque interdum justo eu risus.  Cum sociis natoque
penatibus et magnis dis parturient montes, nascetur ridiculus mus.  Nunc
semper.

In semper enim ut odio.  Nulla varius leo commodo elit.  Quisque condimentum, nisl eget elementum laoreet, mauris turpis elementum felis, ut accumsan nisl velit et mi.

And some Russian: \u041f\u0438\u0442\u043e\u043d - \u043b\u0443\u0447\u0448\u0438\u0439 \u044f\u0437\u044b\u043a \u043f\u0440\u043e\u0433\u0440\u0430\u043c\u043c\u0438\u0440\u043e\u0432\u0430\u043d\u0438\u044f!

"""
            if word_wrap:
                lorem_ipsum = wrap(lorem_ipsum)
            self.stc.ClearAll()
            for i in range(paragraphs):
                self.stc.AppendText(lorem_ipsum)
            wx.CallAfter(self.OnPrintPreview, None)

        def menuAdd(self, menu, name, desc, fcn, id=-1, kind=wx.ITEM_NORMAL):
            if id == -1:
                id = wx.NewId()
            a = wx.MenuItem(menu, id, name, desc, kind)
            menu.AppendItem(a)
            wx.EVT_MENU(self, id, fcn)
            menu.SetHelpString(id, desc)
        
        def OnOpenFile(self, evt):
            dlg = wx.FileDialog(self, "Choose a text file",
                               defaultDir = "",
                               defaultFile = "",
                               wildcard = "*")
            if dlg.ShowModal() == wx.ID_OK:
                print("Opening %s" % dlg.GetPath())
                self.loadFile(dlg.GetPath())
            dlg.Destroy()
        
        def OnQuit(self, evt):
            self.Close(True)
        
        def getPrintData(self):
            return self.print_data
        
        def OnPrintPreview(self, evt):
            wx.CallAfter(self.showPrintPreview)
        
        def showPrintPreview(self):
            printout = STCPrintout(self.stc, title="Testing!!!", border=True, output_point_size=10)
            printout2 = STCPrintout(self.stc, title="Testing!!!", border=True, output_point_size=10)
            preview = wx.PrintPreview(printout, printout2, self.getPrintData())
            preview.SetZoom(100)
            if preview.IsOk():
                pre_frame = wx.PreviewFrame(preview, self, _("Print Preview"))
                dsize = wx.GetDisplaySize()
                pre_frame.SetInitialSize((self.GetSize()[0],
                                          dsize.GetHeight() - 100))
                pre_frame.Initialize()
                pre_frame.Show()
            else:
                wx.MessageBox(_("Failed to create print preview"),
                              _("Print Error"),
                              style=wx.ICON_ERROR|wx.OK)
        
        def OnPrint(self, evt):
            wx.CallAfter(self.showPrint)
        
        def showPrint(self):
            pdd = wx.PrintDialogData(self.getPrintData())
            printer = wx.Printer(pdd)
            printout = STCPrintout(self.stc)
            result = printer.Print(self.stc, printout)
            if result:
                data = printer.GetPrintDialogData()
                self.print_data = wx.PrintData(data.GetPrintData())
            elif printer.GetLastError() == wx.PRINTER_ERROR:
                wx.MessageBox(_("There was an error when printing.\n"
                                "Check that your printer is properly connected."),
                              _("Printer Error"),
                              style=wx.ICON_ERROR|wx.OK)
            printout.Destroy()

    app = wx.App(False)
    frame = Frame(None, size=(800, -1))
    word_wrap = False
    filename = None
    if len(sys.argv) > 1:
        if not sys.argv[-1].startswith("-"):
            filename = sys.argv[-1]
    if '-d' in sys.argv:
        STCPrintout.debuglevel = 1
    if '-w' in sys.argv:
        word_wrap = True
    if filename:
        frame.loadFile(filename, word_wrap)
    else:
        frame.loadSample(word_wrap=word_wrap)
    frame.Show()
    app.MainLoop()
