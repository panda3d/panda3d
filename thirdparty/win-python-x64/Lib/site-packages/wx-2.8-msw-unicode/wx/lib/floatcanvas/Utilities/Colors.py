#!/usr/bin/env python

"""
Colors.py

Assorted stuff for Colors support. At the moment, only a few color sets.

Many of these are from:
http://geography.uoregon.edu/datagraphics/color_scales.htm

They may have been modified some

CategoricalColor1: A list of colors that are distict.
BlueToRed11: 11 colors from blue to red 


"""

## Categorical 12-step scheme, after ColorBrewer 11-step Paired Scheme     
## From: http://geography.uoregon.edu/datagraphics/color_scales.htm
# CategoricalColor1 = [ (255, 191, 127),
#                       (255, 127,   0),
#                       (255, 255, 153),
#                       (255, 255,  50),
#                       (178, 255, 140),
#                       ( 50, 255,   0),
#                       (165, 237, 255),
#                       (25, 178, 255),
#                       (204, 191, 255),
#                       (101,  76, 255),
#                       (255, 153, 191),
#                       (229,  25,  50),
#                       ]

CategoricalColor1 = [ (229,  25,  50),
                      (101,  76, 255),
                      ( 50, 255,   0),
                      (255, 127,   0),
                      (255, 255,  50),
                      (255, 153, 191),
                      (25, 178, 255),
                      (178, 255, 140),
                      (255, 191, 127),
                      (204, 191, 255),
                      (165, 237, 255),
                      (255, 255, 153),
                      ]

RedToBlue11 = [ (165,  0,   33),
                (216,  38,  50), 
                (247, 109,  94), 
                (255, 173, 114), 
                (255, 224, 153), 
                (255, 255, 191), 
                (224, 255, 255), 
                (170, 247, 255), 
                (114, 216, 255), 
                ( 63, 160, 255), 
                ( 38, 76,  255), 
                ]

BlueToDarkRed12 = [( 41,  10, 216),
                   ( 38,  77, 255),
                   ( 63, 160, 255),
                   (114, 217, 255),
                   (170, 247, 255),
                   (224, 255, 255),
                   (255, 255, 191),
                   (255, 224, 153),
                   (255, 173, 114),
                   (247, 109,  94),
                   (216,  38,  50),
                   (165,   0,  33),
                   ]

BlueToDarkRed10 = [( 41,  10, 216),
                   ( 38,  77, 255),
                   ( 63, 160, 255),
                   (114, 217, 255),
                   (170, 247, 255),
                   (255, 224, 153),
                   (255, 173, 114),
                   (247, 109,  94),
                   (216,  38,  50),
                   (165,   0,  33),
                   ]

BlueToDarkRed8 = [( 41,  10, 216),
                   ( 38,  77, 255),
                   ( 63, 160, 255),
                   (114, 217, 255),
                   (255, 173, 114),
                   (247, 109,  94),
                   (216,  38,  50),
                   (165,   0,  33),
                   ]



if __name__ == "__main__":
    import wx
    # tiny test app
    AllSchemes = [("CategoricalColor1", CategoricalColor1),
                  ("RedToBlue11", RedToBlue11),
                  ("BlueToDarkRed12", BlueToDarkRed12),
                  ("BlueToDarkRed10", BlueToDarkRed10),
                  ("BlueToDarkRed8", BlueToDarkRed8)
                  ]
    class TestFrame(wx.Frame):
        def __init__(self, *args, **kwargs):
            wx.Frame.__init__(self, *args, **kwargs)
            Hsizer = wx.BoxSizer(wx.HORIZONTAL)
            for scheme in AllSchemes:
                Sizer = wx.BoxSizer(wx.VERTICAL)
                Sizer.Add(wx.StaticText(self, label=scheme[0]), 0, wx.ALL, 5)
                for c in scheme[1]:
                    w = wx.Window(self, size=(100, 20))
                    w.SetBackgroundColour(wx.Colour(*c))
                    Sizer.Add(w, 0, wx.ALL, 5)
                Hsizer.Add(Sizer, 0, wx.ALL, 5)
            self.SetSizerAndFit(Hsizer)
            self.Show()

    A = wx.App(False)
    F = TestFrame(None)
    A.MainLoop()
    