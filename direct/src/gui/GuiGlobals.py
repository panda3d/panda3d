# GuiGlobals.py : global info for the gui package

from ShowBaseGlobal import *
import GuiManager

guiMgr = GuiManager.GuiManager.getPtr(base.win, base.mak.node(),
                                      base.render2d.node())

font = loader.loadModelNode("models/fonts/ttf-comic")

def getDefaultFont():
    return font

def setDefaultFont(newFont):
    global font
    font = newFont

