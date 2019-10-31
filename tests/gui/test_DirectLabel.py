# coding=utf-8
from direct.gui.DirectLabel import DirectLabel

def test_create_label():
    position = (0.0, 0.0, -0.6)
    text = "hello"
    scale = (0.1, 0.1, 0.1)
    label = DirectLabel(text=text,
                        pos=position,
                        scale=scale,
                        textMayChange=1)
