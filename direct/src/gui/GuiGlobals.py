# GuiGlobals.py : global info for the gui package

from ShowBaseGlobal import *
import GuiManager

guiMgr = GuiManager.GuiManager.getPtr(base.win, base.mak.node(),
                                      base.render2d.node())

font = loader.loadModelNode("models/fonts/ttf-comic")
drawOrder = 100

def getDefaultFont():
    return font

def setDefaultFont(newFont):
    global font
    font = newFont

def getDefaultDrawOrder():
    return drawOrder

def setDefaultDrawOrder(newDrawOrder):
    global drawOrder
    drawOrder = newDrawOrder
