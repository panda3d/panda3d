"""
Palette for Prototyping
"""

from ProtoPaletteBase import *

class ProtoPalette(ProtoPaletteBase):
    def __init__(self):
        self.dirname = os.path.dirname(__file__)
        ProtoPaletteBase.__init__(self)
