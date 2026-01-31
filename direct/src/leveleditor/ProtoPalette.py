"""
Palette for Prototyping
"""
try :   
    from .ProtoPaletteBase import *
except :
    from ProtoPaletteBase import *
import os

class ProtoPalette(ProtoPaletteBase):
    def __init__(self):
        self.dirname = os.path.dirname(__file__)
        ProtoPaletteBase.__init__(self)
