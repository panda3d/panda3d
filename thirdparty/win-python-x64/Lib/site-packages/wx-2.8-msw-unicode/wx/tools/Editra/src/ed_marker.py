###############################################################################
# Name: ed_marker.py                                                          #
# Purpose: StyledTextCtrl Markers                                             #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2011 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
Classes to represent Markers and associated data in a StyledTextCtrl

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: ed_marker.py 67626 2011-04-27 02:51:39Z CJP $"
__revision__ = "$Revision: 67626 $"

#-----------------------------------------------------------------------------#
import wx
import wx.stc
from extern.embeddedimage import PyEmbeddedImage

# NOTE: Must be 1 char per pixel for Scintilla to display
_BookmarkBmp = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAAAXNSR0IArs4c6QAAAAZiS0dE"
    "AP8A/wD/oL2nkwAAAAlwSFlzAAALEwAACxMBAJqcGAAAAAd0SU1FB9sDAQA0GON3MFgAAAEG"
    "SURBVDjL7ZK9SgNBFIW/nd1SO1EbBUHQgD+g+AKSN9E2nY+RztfxOSwCAUEIFlHUsLPB7L3H"
    "YjdjTBqxFG91mLnzzTlzB/4rA+j2+vpJ893tTba8VszF/dU1uQRWg2cU7uAiyMFgfTig2+tr"
    "GRLmIjeDmVGYKGojmBHMoXawmsnuHs8n5ytuE4AyEqoSjxGmEY8VXpUQI0wrqCLvW9uMD4+4"
    "XICkCDZ6BAQmXI19JILUaBMB5y0PTDrHKU4C+OgB97bZmwuCOyaBQ46n/YOX1/SgCfAxHIAc"
    "1LpzgQA5OWJmgMTp2sa3aXxFGD81Qu1BFzlqUzkIznY6K6NMgIvN/V/9gz9Qn2ObnTkNCjcr"
    "AAAAAElFTkSuQmCC")

_ArrowBmp = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAA8AAAAQCAYAAADJViUEAAAAAXNSR0IArs4c6QAAAAZiS0dE"
    "AP8A/wD/oL2nkwAAAAlwSFlzAAALEwAACxMBAJqcGAAAAAd0SU1FB9sDAQEBO+Lj6asAAADR"
    "SURBVCjPpZItEgIxDIUfTA+B3CNEItsbrOAQDIojLA6J4hSI3KDBIeOwOygEzOwNFgP7A/2B"
    "Ia5pvte+eQH+qEmoycytqgIAiAhlWQbnpillEYGqgpnbn+GcgGHmtmmaUXN49n4H59adnaEF"
    "o6oQkeTrMQEzHIgXwXv9EDDvQ2kBwLk+kRF8P+ezPewFi9XT8/DidsnDy62FtRZE1MPHTfzL"
    "szlwPQGV9ODIcyU2CFZWgiAAGCJCURQfUF3XXYQhMLvbLzgEfrWeMTALp0AAeAAUy3GCxymX"
    "vQAAAABJRU5ErkJggg==")

_BreakpointBmp = PyEmbeddedImage(
     "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAAAXNSR0IArs4c6QAAAAZiS0dE"
    "AP8A/wD/oL2nkwAAAAlwSFlzAAALEwAACxMBAJqcGAAAAAd0SU1FB9sDEgMQBqh6qrUAAAEf"
    "SURBVDjLpZPNTsJQEIW/CxgJUStWJTRqEBZEiZqY6AOY2FdiaVz2Adz6IiY+gHFHGn82yI9V"
    "UjUWFkKh1lUx1VKvYZZzc76cOzMHpiwR1axWND+qb5iW+BNQrWj+qrLE/maZQnYFgNeew63V"
    "oGY9/IKIn+Kj3UP0wlak3fvnFudXlyGICIl3DtDzxdg/t95szq6/IalAnF9U0bMa9PuxgPXM"
    "PMcbZQDfMC2RCh6214q89Bypye+peS6ad4wdAKhzCgnXl9vdaMTCzGwYMHCHdPsjaUAwvDGg"
    "030n4yWlAc5wEAbU7UeU9LKUvvPRJXKNpVxJCvBkNzmpNUTkIeWS8S4cr8epWRexp4yXnpAc"
    "F+OmLf4RpgT4gPicGKap6ws0jWfqOADTLwAAAABJRU5ErkJggg==")

_BreakpointDisabledBmp = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAAAXNSR0IArs4c6QAAAAZiS0dE"
    "AP8A/wD/oL2nkwAAAAlwSFlzAAALEwAACxMBAJqcGAAAAAd0SU1FB9sDEgMRGEtuppcAAABy"
    "SURBVDjLY2TAAcq1pf4j8zuvPmPEpo6RGM2EDMHQjM0AXOIkKcIrT5QNuNQRqxmbekYYg6gA"
    "whLQTAwUgoE3gOJAJCsaG3Xl/5OdkOq1FYlIjbqqmElZQ+l/uabMfxIzExMDw38GBgbGfzgz"
    "EyOl2RkAwXRPWcN07zMAAAAASUVORK5CYII=")

_StackMarker = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAABHNCSVQICAgIfAhkiAAAAd1J"
    "REFUOI2Fk09rE1EUxX930jSZJIaBkGJcKBilC0uxG+kkuIhKKdn5EfoREvwO7jprV+LahSuL"
    "CKEQqCO6cJNFQ1qEFBJJKYSaPx1Br4sx00xG64EH79377rnnHe4TMWLMw66LChKKqSquo+Hg"
    "H8iMwK6JrtxNkMvluJ3eJF9chqFF99sxrX6T06MffNj9FSERMWJB1+qjp+RzK2ANaU8aAGST"
    "eQrGOnv7bxh0vIgSw9cI+TvLfO6+5dXrF3QPz1hNPQbg/OKU9qRBdWOHVDKDXRMNEZTqhooI"
    "o5M4g46HqtLqN+kenpFN5i9vWkNu3c8iEn6FMds0nn8X11FxHZVBx6PVb1Iw1um1xwC0Jw0K"
    "y/ciJhqqihJSheuojE7iANxYTQckZvxalADxPVjE1BuHzr322Dd4AUvuru+qXRNNJTPhwqFF"
    "7+s4UlSqG0FLmR8kuyYqIlQePuHm9SIAe19eBvnqxg5AMBuDjhcmmFey+cDmeHrAdPgzyJlW"
    "jLXsNvsH75lcjHAdFYMFuI7KxBvx8ZPLWnYb07psUDTLfrE3CgYqoiBQUhdNJTJUylu0zt9R"
    "NMu+7COPmW9XEszMMhNpKuWtkOz5O1cSzHsy9cZ//UyIEfvvKj1b0n/lfgPWHMMhVFXV8gAA"
    "AABJRU5ErkJggg==")

_ErrorBmp = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAABHNCSVQICAgIfAhkiAAAAc5J"
    "REFUOI11kzFP20AUx3/nEBGLRFUr5CEIieoGhkRtVJVI3uIROiBgYWLzxgco3SulysjIF8iC"
    "UJaspjOtRIEODKZZihBClcABW0I5d2h8shP3P53e6f977929J4RRIK3PahQLIciTEcd8NAqZ"
    "S5EGtNUorklJy3VzAccHB/z0/QxEA9Lm3ycnPPt+xjxvmpTX16cgxqR50O3mZr8LQ4a9Hk3b"
    "piYlbTWKAYzJzEUpebm/T1FKbS5KidXvc1+tonyflutqiKGEoOW6DHs9nn2fhZUVAMzNTYpS"
    "amASuwtDBt0uTdtGCcFMusTkod4sLlJyHEqOo6uIPI+HTmf6ZyYDS5UKD50Okedl4nnmXEDS"
    "czo7wNL2tj4PggA1/qUMYN40Mz1HnqcredrZwarXGQQBszc32jMD8HRxAcCv01MWdne1OV12"
    "yXGIVleZPTqirJSOiy8Q16Skadv4h4cAvFpb40+/D8DQMCgrhdza4sfYXFleJri85DiK/k1i"
    "W43iD40GVr2uIYkSQKLE/DUM2TMKIjPK7+bmeLuxMQVJ9LrR4PzsjO+Pj+yNRzl3md5bFvfV"
    "Ki+urzOAb7e3nF9d8UkY08uUhqj/rLOIY5050V9UfNMzpyji5gAAAABJRU5ErkJggg==")

_LintBmpGreen = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAABHNCSVQICAgIfAhkiAAAAZNJ"
    "REFUOI3Fk00ow2Ecxz/P/ogtYxNRaDIvlykvO4yThHJzUBx3lLujg6O75OQkB4lSJKSUVsMO"
    "qI1lW15aJv5/85KXbX8na48R5eBbz+H5Pd/ft2/f3+8RwqDwFxj+1P2TQMd8q94y1qgDdM+5"
    "9F8JOGeaJGKBw5SufYhlIudzoTjfhHPSoSev3jAm8njKecWWW0m/vZfVuw18BCS++CrED7tu"
    "+yAAi8draIpKf0kfs3dLAOwM7AtJoHvOpd/4VQzPOpZmKz3l7QBEosG0sFcNk7x6wzceEFkZ"
    "rA95RIHDhKXZits+SCQaxHvpJ5ZKYKuoo6OuCwClJk9yK4X4svtAcdLCxXWYWCoBQGeZi0g0"
    "iKbGuT3SuPWq1A5XpcOUQkzlCzRFTd+d1no8IS9ms2DxZgVLoZG9iWOR2SM58I0HhPb8yHx4"
    "k84yF4dnpziqa4nHdULbFwgla2jZe7DrPhAYBVsxD2azYHp5AX/ohNOpcwHQNtog7cKXY8wk"
    "qvdPFJWapOQlCIPy47GP2PRv3//9N74DHQaLgsqg1M8AAAAASUVORK5CYII=")

_LintBmpRed = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAABHNCSVQICAgIfAhkiAAAAXJJ"
    "REFUOI3Fk79Lw0AYhp9LhA4tFIyQQSg2oq2zRTPYrtLJjpLRVf8AxVnwD9C1o4OTBcVdC42i"
    "jtpWjCIVGq1xsYiEek4NptQf0MEPDj7uvve9l+c4IRSVQUoZSP2bQbmwILdnZ2S3/5PBQS4b"
    "DLZcl1FFYcVI9hUDDPVuxIeHKWam5ZuqMqLrACzqOgnLothoyKWzc/F1XvSD2I2bsCwA7nd2"
    "ANBMk6ejI7yXFwoVW4QMyoUFeVirAZBPpdBMk/fHR14dh5hh8Oo4tFyXh48Plk9OgxQBg7m9"
    "kphPJsmnUiQsKxB3SzNNAMYikVDaEMTdahUAz7YDsWaaQe/5PsfNJsXMdAA1BDHe4x4zDJ5t"
    "G4BaqcRNu81m/ToEMZRg46oq9i8vuatUgptjhkHLdfF8n06nQ2/1fYWDXFb6vs+IrlNvNLjw"
    "PLacW7E6OSFVVWXjqip+NABYn0pLgPFolJt2OyQKR1DUX9daOi2/Pf/33/gJL9aOTtHLtUIA"
    "AAAASUVORK5CYII=")

_LintBmpYellow = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAABHNCSVQICAgIfAhkiAAAAblJ"
    "REFUOI3FkzFIG2EUx39frmCStgpqIw0pJKGEpErViEIHoYtQcOjSTkJ1ky7irLM6ScVFbBeL"
    "FBxEBAUHl6IirVMKFdseXE5yjSVGCzW53ImXr0Mh9DRiwaEPvuG9773/e/y+9wnhUbiOea5V"
    "fZXA7FibHB6ISoD5yQ75TwLPntytJOpGEZ9PYag/Ik3LqdrkxvnA054gj5INMpM1CYduUiie"
    "8eB+LV3dfYwYphyf+Sb+zhfVIM5PdkjTcnjc8wKAnc13AMQTrWxs7ZD/aTMxowqXwOxYm9xT"
    "f1GWks6H9cQTrZxax2jpDIHGGgDefzikVHKYmtMqU1QYDI6mxL2gn+ZYHV3dfRwffkFLZwC4"
    "dTtA/Z04AMaP0uUQ978X8XsV8sYGumFiWg6xeAuf93TK0kHVCyBg5GWsAtoF0SMEuSObaOSP"
    "Hw75SaU+4fcqLK9uEwx4mX6bFnBQfYKpOU181U5YXd8l2Z4kl7dpSYTJHdmoegHLLl8AfmEP"
    "Xi/sCzVdYGllk0BjDa/efCSTNVlcOxAAQ/0R10JVfUaA4YGoLEtJNmcRavK5yLtMeJQrz/Pe"
    "kLz0/r//xt8H26DcJVZnmgAAAABJRU5ErkJggg==")

#-----------------------------------------------------------------------------#

__markerId = -1
def NewMarkerId():
    """Get a new marker id
    @note: limited by stc to 16 possible ids. will assert when this threshold
           is passed.

    """
    global __markerId
    __markerId += 1
    assert __markerId < 24, "No more marker Ids available!"
    return __markerId

#-----------------------------------------------------------------------------#

class Marker(object):
    """Marker Base class"""
    _ids = list()
    _symbols = list()
    def __init__(self):
        super(Marker, self).__init__()

        # Attributes
        self._line = -1
        self._handle = -1
        self._bmp = wx.NullBitmap
        self._fore = wx.NullColour   # Foreground colour
        self._back = wx.NullColour   # Background colour

    Line = property(lambda self: self._line,
                    lambda self, line: setattr(self, '_line', line))
    Handle = property(lambda self: self._handle,
                      lambda self, handle: setattr(self, '_handle', handle))
    Bitmap = property(lambda self: self._bmp,
                      lambda self, bmp: setattr(self, '_bmp', bmp))
    Foreground = property(lambda self: self._fore,
                          lambda self, fore: setattr(self, '_fore', fore))
    Background = property(lambda self: self._back,
                          lambda self, back: setattr(self, '_back', back))

    @classmethod
    def AnySet(cls, stc, line):
        """Is any breakpoint set on the line"""
        if not cls.IsSet(stc, line):
            # Check subclasses
            for bpoint in cls.__subclasses__():
                if bpoint.IsSet(stc, line):
                    return True
            return False
        else:
            return True

    @classmethod
    def GetIds(cls):
        """Get the list of marker IDs."""
        return cls._ids

    @classmethod
    def GetSymbols(cls):
        """Get the list of symbols"""
        return cls._symbols

    @classmethod
    def IsSet(cls, stc, line):
        """Is the marker set on the given line"""
        mask = stc.MarkerGet(line)
        return True in [ bool(1<<marker & mask) for marker in cls.GetIds() ]

    def Set(self, stc, line, delete=False):
        """Add/Delete the marker to the stc at the given line"""
        for marker in self.GetIds():
            if delete:
                mask = stc.MarkerGet(line)
                if (1<<marker & mask):
                    stc.MarkerDelete(line, marker)
            else:
                handle = stc.MarkerAdd(line, marker)
                if self.Handle < 0:
                    self.Line = line
                    self.Handle = handle

    def DeleteAll(self, stc):
        """Remove all instances of this bookmark from the stc"""
        for marker in self.GetIds():
            stc.MarkerDeleteAll(marker)

    def RegisterWithStc(self, stc):
        """Setup the STC to use this marker"""
        ids = self.GetIds()
        if self.Bitmap.IsNull():
            symbols = self.GetSymbols()
            if len(ids) == len(symbols):
                markers = zip(ids, symbols)
                for marker, symbol in markers:
                    stc.MarkerDefine(marker, symbol,
                                     self.Foreground, self.Background)
        elif len(ids) == 1 and not self.Bitmap.IsNull():
            stc.MarkerDefineBitmap(ids[0], self.Bitmap)
        else:
            assert False, "Invalid Marker!"

#-----------------------------------------------------------------------------#

class Bookmark(Marker):
    """Class to store bookmark data"""
    _ids = [NewMarkerId(),]
    def __init__(self):
        super(Bookmark, self).__init__()

        # Attributes
        self._name = u""        # Bookmark alias name
        self._fname = u""       # Filename
        self.Bitmap = _BookmarkBmp.Bitmap

    def __eq__(self, other):
        return (self.Filename, self.Line) == (other.Filename, other.Line)

    #---- Properties ----#
    Name = property(lambda self: self._name,
                    lambda self, name: setattr(self, '_name', name))
    Filename = property(lambda self: self._fname,
                        lambda self, name: setattr(self, '_fname', name))

#-----------------------------------------------------------------------------#

class Breakpoint(Marker):
    """Marker object to represent a breakpoint in the EditraBaseStc"""
    _ids = [NewMarkerId(),]
    def __init__(self):
        super(Breakpoint, self).__init__()
        self.Bitmap = _BreakpointBmp.Bitmap

class BreakpointDisabled(Breakpoint):
    """Marker object to represent a disabled breakpoint in the EditraBaseStc"""
    _ids = [NewMarkerId(),]
    def __init__(self):
        super(BreakpointDisabled, self).__init__()
        self.Bitmap = _BreakpointDisabledBmp.Bitmap

class BreakpointStep(Breakpoint):
    """Marker object to represent debugger step breakpoint in the EditraBaseStc"""
    _ids = [NewMarkerId(), NewMarkerId()]
    def __init__(self):
        super(BreakpointStep, self).__init__()
        self.Bitmap = _ArrowBmp.Bitmap

    def DeleteAll(self, stc):
        """Overrode to handle refresh issue"""
        super(BreakpointStep, self).DeleteAll(stc)
        stc.Colourise(0, stc.GetLength())

    def RegisterWithStc(self, stc):
        """Register this compound marker with the given StyledTextCtrl"""
        ids = self.GetIds()
        stc.MarkerDefineBitmap(ids[0], self.Bitmap)
        stc.MarkerDefine(ids[1], wx.stc.STC_MARK_BACKGROUND, 
                         background=self.Background)

    def Set(self, stc, line, delete=False):
        """Add/Delete the marker to the stc at the given line
        @note: overrode to ensure only one is set in a buffer at a time

        """
        self.DeleteAll(stc)
        super(BreakpointStep, self).Set(stc, line, delete)
        start = stc.GetLineEndPosition(max(line-1, 0))
        end = stc.GetLineEndPosition(line)
        if start == end:
            start = 0
        stc.Colourise(start, end) # Refresh for background marker

class StackMarker(Marker):
    """Marker object to mark a line in a callstack in the EditraBaseStc"""
    _ids = [NewMarkerId(),]
    def __init__(self):
        super(StackMarker, self).__init__()
        self.Bitmap = _StackMarker.Bitmap

#-----------------------------------------------------------------------------#

class FoldMarker(Marker):
    """Marker object class for managing the code folding markers"""
    _ids = [wx.stc.STC_MARKNUM_FOLDEROPEN, wx.stc.STC_MARKNUM_FOLDER,
            wx.stc.STC_MARKNUM_FOLDERSUB, wx.stc.STC_MARKNUM_FOLDERTAIL,
            wx.stc.STC_MARKNUM_FOLDEREND, wx.stc.STC_MARKNUM_FOLDEROPENMID,
            wx.stc.STC_MARKNUM_FOLDERMIDTAIL]
    _symbols = [wx.stc.STC_MARK_BOXMINUS, wx.stc.STC_MARK_BOXPLUS, 
                wx.stc.STC_MARK_VLINE, wx.stc.STC_MARK_LCORNER, 
                wx.stc.STC_MARK_BOXPLUSCONNECTED, 
                wx.stc.STC_MARK_BOXMINUSCONNECTED, wx.stc.STC_MARK_TCORNER]

    def RegisterWithStc(self, stc):
        super(FoldMarker, self).RegisterWithStc(stc)
        stc.SetFoldMarginHiColour(True, self.Foreground)
        stc.SetFoldMarginColour(True, self.Foreground)

#-----------------------------------------------------------------------------#

class ErrorMarker(Marker):
    """Marker object to indicate an error line in the EditraBaseStc"""
    _ids = [NewMarkerId(), NewMarkerId()]
    def __init__(self):
        super(ErrorMarker, self).__init__()
        self.Bitmap = _ErrorBmp.Bitmap

    def DeleteAll(self, stc):
        """Overrode to handle refresh issue"""
        super(ErrorMarker, self).DeleteAll(stc)
        stc.Colourise(0, stc.GetLength())

    def RegisterWithStc(self, stc):
        """Register this compound marker with the given StyledTextCtrl"""
        ids = self.GetIds()
        stc.MarkerDefineBitmap(ids[0], self.Bitmap)
        stc.MarkerDefine(ids[1], wx.stc.STC_MARK_BACKGROUND, 
                         # foreground=self.Foreground, #TODO
                         background=self.Foreground)

    def Set(self, stc, line, delete=False):
        """Add/Delete the marker to the stc at the given line
        @note: overrode to ensure only one is set in a buffer at a time

        """
        super(ErrorMarker, self).Set(stc, line, delete)
        start = stc.GetLineEndPosition(max(line-1, 0))
        end = stc.GetLineEndPosition(line)
        if start == end:
            start = 0
        stc.Colourise(start, end) # Refresh for background marker

#-----------------------------------------------------------------------------#

class LintMarker(Marker):
    """Marker object to represent a marker for coding issue in the EditraBaseStc"""
    _ids = [NewMarkerId(),]
    def __init__(self):
        super(LintMarker, self).__init__()
        self.Bitmap = _LintBmpGreen.Bitmap

class LintMarkerWarning(Marker):
    """Marker object to represent a marker for moderate severity 
    coding issue in the EditraBaseStc

    """
    _ids = [NewMarkerId(),]
    def __init__(self):
        super(LintMarkerWarning, self).__init__()
        self.Bitmap = _LintBmpYellow.Bitmap

class LintMarkerError(Marker):
    """Marker object to represent a marker for a high severity
    coding issue in the EditraBaseStc

    """
    _ids = [NewMarkerId(),]
    def __init__(self):
        super(LintMarkerError, self).__init__()
        self.Bitmap = _LintBmpRed.Bitmap
