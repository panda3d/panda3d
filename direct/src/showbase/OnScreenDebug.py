"""Contains the OnScreenDebug class."""

__all__ = ['OnScreenDebug']

from panda3d.core import *

from direct.gui import OnscreenText
from direct.directtools import DirectUtil

class OnScreenDebug:

    enabled = ConfigVariableBool("on-screen-debug-enabled", False)

    def __init__(self):
        self.onScreenText = None
        self.frame = 0
        self.text = ""
        self.data = {}

    def load(self):
        if self.onScreenText:
            return

        fontPath = ConfigVariableString("on-screen-debug-font", "cmtt12").value
        fontScale = ConfigVariableDouble("on-screen-debug-font-scale", 0.05).value

        color = {
            "black": Vec4(0, 0, 0, 1),
            "white": Vec4(1, 1, 1, 1),
            }
        fgColor = color[ConfigVariableString("on-screen-debug-fg-color", "white").value]
        bgColor = color[ConfigVariableString("on-screen-debug-bg-color", "black").value]
        fgColor.setW(ConfigVariableDouble("on-screen-debug-fg-alpha", 0.85).value)
        bgColor.setW(ConfigVariableDouble("on-screen-debug-bg-alpha", 0.85).value)

        font = loader.loadFont(fontPath)
        if not font.isValid():
            print("failed to load OnScreenDebug font %s" % fontPath)
            font = TextNode.getDefaultFont()
        self.onScreenText = OnscreenText.OnscreenText(
                pos = (-1.0, 0.9), fg=fgColor, bg=bgColor,
                scale = (fontScale, fontScale, 0.0), align = TextNode.ALeft,
                mayChange = 1, font = font)
        # Make sure readout is never lit or drawn in wireframe
        DirectUtil.useDirectRenderStyle(self.onScreenText)

    def render(self):
        if not self.enabled:
            return
        if not self.onScreenText:
            self.load()
        self.onScreenText.clearText()
        entries = list(self.data.items())
        entries.sort()
        for k, v in entries:
            if v[0] == self.frame:
                # It was updated this frame (key equals value):
                #isNew = " is"
                isNew = "="
            else:
                # This data is not for the current
                # frame (key roughly equals value):
                #isNew = "was"
                isNew = "~"
            value = v[1]
            if type(value) == float:
                value = "% 10.4f"%(value,)
            # else: other types will be converted to str by the "%s"
            self.onScreenText.appendText("%20s %s %-44s\n"%(k, isNew, value))
        self.onScreenText.appendText(self.text)
        self.frame += 1

    def clear(self):
        self.text = ""
        if self.onScreenText:
            self.onScreenText.clearText()

    def add(self, key, value):
        self.data[key] = (self.frame, value)
        return 1 # to allow assert onScreenDebug.add("foo", bar)

    def has(self, key):
        return key in self.data

    def remove(self, key):
        del self.data[key]

    def removeAllWithPrefix(self, prefix):
        toRemove = []
        for key in list(self.data.keys()):
            if len(key) >= len(prefix):
                if key[:len(prefix)] == prefix:
                    toRemove.append(key)
        for key in toRemove:
            self.remove(key)

    def append(self, text):
        self.text += text
