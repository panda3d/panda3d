# GuiGlobals.py : global info for the gui package

from ShowBaseGlobal import *
import GuiManager

guiMgr = GuiManager.GuiManager.getPtr(base.win, base.mak.node(),
                                      base.render2d.node())

font = None
panel = None
drawOrder = 100

def getDefaultFont():
    global font
    if font == None:
        font = loader.loadModelNode("models/fonts/Comic")
    return font

def setDefaultFont(newFont):
    global font
    font = newFont

def getDefaultPanel():
    return panel

def setDefaultPanel(newPanel):
    global panel
    panel = newPanel

def getDefaultDrawOrder():
    return drawOrder

def setDefaultDrawOrder(newDrawOrder):
    global drawOrder
    drawOrder = newDrawOrder
