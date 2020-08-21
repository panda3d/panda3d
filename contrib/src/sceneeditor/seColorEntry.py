#################################################################
# seColorEntry.py
# Originally from VectorWidgets.py
# Altered by Yi-Hong Lin, yihhongl@andrew.cmu.edu, 2004
#
# Here we need some widget to handle some color input.
# There is some colorEntry in the VectorWidgets.py, but they didn't
# work as we need (we don't need alpha here. so the dim option should be set to 3).
# So we make one from them.
#
#################################################################
from direct.tkwidgets import Valuator
from direct.tkwidgets import Floater
from direct.tkwidgets import Slider
import sys, Pmw
from direct.tkwidgets.VectorWidgets import VectorEntry

if sys.version_info >= (3, 0):
    from tkinter.colorchooser import askcolor
else:
    from tkColorChooser import askcolor


class seColorEntry(VectorEntry):
    def __init__(self, parent = None, **kw):
        # Initialize options for the class (overriding some superclass options)
        optiondefs = (
            ('dim',                     3,                  Pmw.INITOPT),
            ('type',                    'slider',           Pmw.INITOPT),
            ('fGroup_labels',           ('R','G','B'),      None),
            ('min',                     0.0,                None),
            ('max',                     255.0,              None),
            ('nuDigits',                0,                  None),
            ('valuator_resolution',     1.0,                None),
            )
        self.defineoptions(kw, optiondefs)

        # Initialize the superclass, make sure dim makes it to superclass
        VectorEntry.__init__(self, parent, dim = self['dim'])
        # Add menu item to popup color picker
        self.addMenuItem(
            'Popup color picker',
            command = lambda s = self: s.popupColorPicker())
        # Needed because this method checks if self.__class__ is myClass
        # where myClass is the argument passed into inialiseoptions
        self.initialiseoptions(seColorEntry)

    def popupColorPicker(self):
        # Can pass in current color with: color = (255, 0, 0)
        color = askcolor(
            parent = self.interior(),
            # Initialize it to current color
            initialcolor = tuple(self.get()[:3]))[0]
        if color:
            self.set((color[0], color[1], color[2]))
