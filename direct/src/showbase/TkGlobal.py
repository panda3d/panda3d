""" This module is now vestigial.  """

import sys, Pmw

if sys.version_info >= (3, 0):
    from tkinter import *
else:
    from Tkinter import *

# This is required by the ihooks.py module used by Squeeze (used by
# pandaSqueezer.py) so that Pmw initializes properly
if '_Pmw' in sys.modules:
    sys.modules['_Pmw'].__name__ = '_Pmw'

# Don't export this from the module.
del sys

# Hack to workaround broken Pmw.NoteBook in Python 3
def bordercolors(root, colorName):
    lightRGB = []
    darkRGB = []
    for value in Pmw.Color.name2rgb(root, colorName, 1):
        value40pc = (14 * value) // 10
        if value40pc > int(Pmw.Color._MAX_RGB):
            value40pc = int(Pmw.Color._MAX_RGB)
        valueHalfWhite = (int(Pmw.Color._MAX_RGB) + value) // 2;
        lightRGB.append(max(value40pc, valueHalfWhite))

        darkValue = (60 * value) // 100
        darkRGB.append(darkValue)

    return (
        '#%04x%04x%04x' % (lightRGB[0], lightRGB[1], lightRGB[2]),
        '#%04x%04x%04x' % (darkRGB[0], darkRGB[1], darkRGB[2])
    )

Pmw.Color.bordercolors = bordercolors
del bordercolors


def spawnTkLoop():
    """Alias for :meth:`base.spawnTkLoop() <.ShowBase.spawnTkLoop>`."""
    base.spawnTkLoop()
