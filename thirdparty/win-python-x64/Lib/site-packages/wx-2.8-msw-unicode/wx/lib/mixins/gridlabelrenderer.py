#----------------------------------------------------------------------
# Name:        wx.lib.mixins.gridlabelrenderer
# Purpose:     A Grid mixin that enables renderers to be plugged in
#              for drawing the row and col labels, similar to how the
#              cell renderers work.
#
# Author:      Robin Dunn
#
# Created:     20-Mar-2009
# RCS-ID:      $Id: gridlabelrenderer.py 67478 2011-04-13 18:25:25Z RD $
# Copyright:   (c) 2009 by Total Control Software
# Licence:     wxWindows license
#----------------------------------------------------------------------

"""
A Grid mixin that enables renderers to be plugged in for drawing the
row and col labels, similar to how the cell renderers work.
"""

import wx


class GridWithLabelRenderersMixin(object):
    """
    This class can be mixed with wx.grid.Grid to add the ability to plugin
    label renderer objects for the row, column and corner labels, similar to
    how the cell renderers work in the main Grid class.
    """
    def __init__(self):
        self.GetGridRowLabelWindow().Bind(wx.EVT_PAINT, self._onPaintRowLabels)        
        self.GetGridColLabelWindow().Bind(wx.EVT_PAINT, self._onPaintColLabels)
        self.GetGridCornerLabelWindow().Bind(wx.EVT_PAINT, self._onPaintCornerLabel)
        
        self._rowRenderers = dict()
        self._colRenderers = dict()
        self._cornderRenderer = None
        self._defRowRenderer = None
        self._defColRenderer = None
        
        
    def SetRowLabelRenderer(self, row, renderer):
        """
        Register a renderer to be used for drawing the label for the
        given row.
        """
        if renderer is None:
            if row in self._rowRenderers:
                del self._rowRenderers[row]
        else:
            self._rowRenderers[row] = renderer

            
    def SetDefaultRowLabelRenderer(self, renderer):
        """
        Set the row label renderer that should be used for any row
        that does not have an explicitly set renderer.  Defaults to
        an instance of `GridDefaultRowLabelRenderer`.
        """
        self._defRowRenderer = renderer

    
    def SetColLabelRenderer(self, col, renderer):
        """
        Register a renderer to be used for drawing the label for the
        given column.
        """
        if renderer is None:
            if col in self._colRenderers:
                del self._colRenderers[col]
        else:
            self._colRenderers[col] = renderer

            
    def SetDefaultColLabelRenderer(self, renderer):
        """
        Set the column label renderer that should be used for any
        column that does not have an explicitly set renderer.
        Defaults to an instance of `GridDefaultColLabelRenderer`.
        """
        self._defColRenderer = renderer


    
    def SetCornerLabelRenderer(self, renderer):
        """
        Sets the renderer that should be used for drawing the area in
        the upper left corner of the Grid, between the row labels and
        the column labels.  Defaults to an instance of
        `GridDefaultCornerLabelRenderer`
        """
        self._cornderRenderer = renderer
    
        
    #----------------------------------------------------------------
    
    def _onPaintRowLabels(self, evt):
        window = evt.GetEventObject()
        dc = wx.PaintDC(window)
                
        rows = self.CalcRowLabelsExposed(window.GetUpdateRegion())
        if rows == [-1]:
            return

        x, y = self.CalcUnscrolledPosition((0,0))
        pt = dc.GetDeviceOrigin()
        dc.SetDeviceOrigin(pt.x, pt.y-y)
        for row in rows:
            top, bottom = self._getRowTopBottom(row)
            rect = wx.Rect()
            rect.top = top
            rect.bottom = bottom
            rect.x = 0
            rect.width = self.GetRowLabelSize()

            renderer = self._rowRenderers.get(row, None) or \
                       self._defRowRenderer or GridDefaultRowLabelRenderer()
            renderer.Draw(self, dc, rect, row)
            
            
    def _onPaintColLabels(self, evt):
        window = evt.GetEventObject()
        dc = wx.PaintDC(window)

        cols = self.CalcColLabelsExposed(window.GetUpdateRegion())
        if cols == [-1]:
            return

        x, y = self.CalcUnscrolledPosition((0,0))
        pt = dc.GetDeviceOrigin()
        dc.SetDeviceOrigin(pt.x-x, pt.y)
        for col in cols:
            left, right = self._getColLeftRight(col)
            rect = wx.Rect()
            rect.left = left
            rect.right = right
            rect.y = 0
            rect.height = self.GetColLabelSize()

            renderer = self._colRenderers.get(col, None) or \
                       self._defColRenderer or GridDefaultColLabelRenderer()
            renderer.Draw(self, dc, rect, col)

            
    def _onPaintCornerLabel(self, evt):
        window = evt.GetEventObject()
        dc = wx.PaintDC(window)
        w, h = window.GetSize()
        rect = wx.Rect(0, 0, w, h)
        
        renderer = self._cornderRenderer or GridDefaultCornerLabelRenderer()
        renderer.Draw(self, dc, rect, -1)

        
        
    # NOTE: These helpers or something like them should probably be publicly
    # available in the C++ wxGrid class, but they are currently protected so
    # for now we will have to calculate them ourselves.
    def _getColLeftRight(self, col):
        c = 0
        left = 0
        while c < col:
            left += self.GetColSize(c)
            c += 1
        right = left + self.GetColSize(col)
        return left, right
        
    def _getRowTopBottom(self, row):
        r = 0
        top = 0
        while r < row:
            top += self.GetRowSize(r)
            r += 1
        bottom = top + self.GetRowSize(row) - 1
        return top, bottom
        



class GridLabelRenderer(object):
    """
    Base class for row, col or corner label renderers.
    """
    
    def Draw(self, grid, dc, rect, row_or_col):
        """
        Override this method in derived classes to do the actual
        drawing of the label.
        """
        raise NotImplementedError

    
    # These two can be used to duplicate the default wxGrid label drawing
    def DrawBorder(self, grid, dc, rect):
        """
        Draw a standard border around the label, to give a simple 3D
        effect like the stock wx.grid.Grid labels do.
        """
        top = rect.top
        bottom = rect.bottom
        left = rect.left
        right = rect.right        
        dc.SetPen(wx.Pen(wx.SystemSettings.GetColour(wx.SYS_COLOUR_3DSHADOW)))
        dc.DrawLine(right, top, right, bottom)
        dc.DrawLine(left, top, left, bottom)
        dc.DrawLine(left, bottom, right, bottom)
        dc.SetPen(wx.WHITE_PEN)
        dc.DrawLine(left+1, top, left+1, bottom)
        dc.DrawLine(left+1, top, right, top)

        
    def DrawText(self, grid, dc, rect, text, hAlign, vAlign):
        """
        Draw the label's text in the rectangle, using the alignment
        flags, and the grid's specified label font and color.
        """
        dc.SetBackgroundMode(wx.TRANSPARENT)
        dc.SetTextForeground(grid.GetLabelTextColour())
        dc.SetFont(grid.GetLabelFont())
        rect = wx.Rect(*rect)
        rect.Deflate(2,2)
        grid.DrawTextRectangle(dc, text, rect, hAlign, vAlign)
        


# These classes draw approximately the same things that the built-in
# label windows do in C++, but are adapted to fit into this label
# renderer scheme.

class GridDefaultRowLabelRenderer(GridLabelRenderer):
    def Draw(self, grid, dc, rect, row):
        hAlign, vAlign = grid.GetRowLabelAlignment()
        text = grid.GetRowLabelValue(row)
        self.DrawBorder(grid, dc, rect)
        self.DrawText(grid, dc, rect, text, hAlign, vAlign)
        
class GridDefaultColLabelRenderer(GridLabelRenderer):
    def Draw(self, grid, dc, rect, col):
        hAlign, vAlign = grid.GetColLabelAlignment()
        text = grid.GetColLabelValue(col)
        self.DrawBorder(grid, dc, rect)
        self.DrawText(grid, dc, rect, text, hAlign, vAlign)
        
class GridDefaultCornerLabelRenderer(GridLabelRenderer):
    def Draw(self, grid, dc, rect, row_or_col):
        self.DrawBorder(grid, dc, rect)
        
        
#---------------------------------------------------------------------------
