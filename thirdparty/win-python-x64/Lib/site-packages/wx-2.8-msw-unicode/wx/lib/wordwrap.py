#----------------------------------------------------------------------
# Name:        wx.lib.wordwrap
# Purpose:     Contains a function to aid in word-wrapping some text
#
# Author:      Robin Dunn
#
# Created:     15-Oct-2006
# RCS-ID:      $Id: wordwrap.py 64235 2010-05-06 18:20:36Z RD $
# Copyright:   (c) 2006 by Total Control Software
# Licence:     wxWindows license
#----------------------------------------------------------------------

def wordwrap(text, width, dc, breakLongWords=True, margin=0):
    """
    Returns a copy of text with newline characters inserted where long
    lines should be broken such that they will fit within the given
    width, with the given margin left and right, on the given `wx.DC`
    using its current font settings.  By default words that are wider
    than the margin-adjusted width will be broken at the nearest
    character boundary, but this can be disabled by passing ``False``
    for the ``breakLongWords`` parameter.
    """

    wrapped_lines = []
    text = text.split('\n')
    for line in text:
        pte = dc.GetPartialTextExtents(line)        
        wid = ( width - (2*margin+1)*dc.GetTextExtent(' ')[0] 
              - max([0] + [pte[i]-pte[i-1] for i in range(1,len(pte))]) )
        idx = 0
        start = 0
        startIdx = 0
        spcIdx = -1
        while idx < len(pte):
            # remember the last seen space
            if line[idx] == ' ':
                spcIdx = idx

            # have we reached the max width?
            if pte[idx] - start > wid and (spcIdx != -1 or breakLongWords):
                if spcIdx != -1:
                    idx = min(spcIdx + 1, len(pte) - 1)
                wrapped_lines.append(' '*margin + line[startIdx : idx] + ' '*margin)
                start = pte[idx]
                startIdx = idx
                spcIdx = -1

            idx += 1

        wrapped_lines.append(' '*margin + line[startIdx : idx] + ' '*margin)

    return '\n'.join(wrapped_lines)



if __name__ == '__main__':
    import wx
    class TestPanel(wx.Panel):
        def __init__(self, parent):
            wx.Panel.__init__(self, parent)

            self.tc = wx.TextCtrl(self, -1, "", (20,20), (150,150), wx.TE_MULTILINE)
            self.Bind(wx.EVT_TEXT, self.OnDoUpdate, self.tc)
            self.Bind(wx.EVT_SIZE, self.OnSize)
            

        def OnSize(self, evt):
            wx.CallAfter(self.OnDoUpdate, None)
            
            
        def OnDoUpdate(self, evt):
            WIDTH = self.GetSize().width - 220
            HEIGHT = 200
            bmp = wx.EmptyBitmap(WIDTH, HEIGHT)
            mdc = wx.MemoryDC(bmp)
            mdc.SetBackground(wx.Brush("white"))
            mdc.Clear()
            mdc.SetPen(wx.Pen("black"))
            mdc.SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.NORMAL))
            mdc.DrawRectangle(0,0, WIDTH, HEIGHT)

            text = wordwrap(self.tc.GetValue(), WIDTH-2, mdc, False)
            #print repr(text)
            mdc.DrawLabel(text, (1,1, WIDTH-2, HEIGHT-2))

            del mdc
            dc = wx.ClientDC(self)
            dc.DrawBitmap(bmp, 200, 20)


    app = wx.App(False)
    frm = wx.Frame(None, title="Test wordWrap")
    pnl = TestPanel(frm)
    frm.Show()
    app.MainLoop()


