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

def getDefaultRolloverSound():
    return base.loadSfx("phase_3/audio/sfx/GUI_rollover.mp3")

def getDefaultClickSound():
    return base.loadSfx("phase_3/audio/sfx/GUI_create_toon_fwd.mp3")

def getNewRolloverFunctor(sound = None):
    val = None
    if sound:
        roll = sound
    else:
        roll = base.loadSfx("phase_3/audio/sfx/GUI_rollover.mp3")
    if roll:
        val = AudioGuiFunctor(roll)
    else:
        val = AudioGuiFunctor()
    return val

def getNewClickFunctor(sound = None):
    val = None
    if sound:
        click = sound
    else:
        click = base.loadSfx("phase_3/audio/sfx/GUI_create_toon_fwd.mp3")
    if click:
        val = AudioGuiFunctor(click)
    else:
        val = AudioGuiFunctor()
    return val
