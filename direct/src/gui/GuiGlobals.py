# GuiGlobals.py : global info for the gui package

from ShowBaseGlobal import *

font = loader.loadModelNode("models/fonts/ttf-comic")

def getDefaultFont():
    return font

def setDefaultFont(newFont):
    global font
    font = newFont
