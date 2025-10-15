"""Contains the OnScreenDebug class."""

from __future__ import annotations

__all__ = ['OnScreenDebug']

from typing import Any

from panda3d.core import (
    ConfigVariableBool,
    ConfigVariableDouble,
    ConfigVariableString,
    TextNode,
    Vec4,
)

from direct.gui import OnscreenText
from direct.directtools import DirectUtil


class OnScreenDebug:

    enabled = ConfigVariableBool("on-screen-debug-enabled", False)

    def __init__(self) -> None:
        self.onScreenText: OnscreenText.OnscreenText | None = None
        self.frame = 0
        self.text = ""
        self.data: dict[str, tuple[int, Any]] = {}

    def load(self) -> None:
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

        from direct.showbase.ShowBaseGlobal import base
        font = base.loader.loadFont(fontPath)
        if not font.isValid():
            print("failed to load OnScreenDebug font %s" % fontPath)
            font = TextNode.getDefaultFont()
        self.onScreenText = OnscreenText.OnscreenText(
                parent = base.a2dTopLeft, pos = (0.0, -0.1),
                fg=fgColor, bg=bgColor, scale = (fontScale, fontScale, 0.0),
                align = TextNode.ALeft, mayChange = True, font = font)
        # Make sure readout is never lit or drawn in wireframe
        DirectUtil.useDirectRenderStyle(self.onScreenText)

    def render(self) -> None:
        if not self.enabled:
            return
        if not self.onScreenText:
            self.load()
        assert self.onScreenText is not None
        self.onScreenText.clearText()
        for k, v in sorted(self.data.items()):
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
            if isinstance(value, float):
                value = "% 10.4f"%(value,)
            # else: other types will be converted to str by the "%s"
            self.onScreenText.appendText("%20s %s %-44s\n"%(k, isNew, value))
        self.onScreenText.appendText(self.text)
        self.frame += 1

    def clear(self) -> None:
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
