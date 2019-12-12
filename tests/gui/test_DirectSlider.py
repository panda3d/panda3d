from direct.gui.DirectSlider import DirectSlider
from direct.gui import DirectGuiGlobals as DGG
import pytest

def test_slider_orientation():
    slider = DirectSlider()

    # Horizontal orientation is the default
    assert slider['orientation'] == DGG.HORIZONTAL
    assert slider['frameSize'] == (-1, 1, -0.08, 0.08)
    assert slider['frameVisibleScale'] == (1, 0.25)

    # try change to vertical orientation
    slider['orientation'] = DGG.VERTICAL
    assert slider['orientation'] == DGG.VERTICAL
    assert slider['frameSize'] == (-0.08, 0.08, -1, 1)
    assert slider['frameVisibleScale'] == (0.25, 1)

    # back to horizontal
    slider['orientation'] = DGG.HORIZONTAL
    assert slider['orientation'] == DGG.HORIZONTAL
    assert slider['frameSize'] == (-1, 1, -0.08, 0.08)
    assert slider['frameVisibleScale'] == (1, 0.25)

    # finally change to inverted vertical orientation
    slider['orientation'] = DGG.VERTICAL_INVERTED
    assert slider['orientation'] == DGG.VERTICAL_INVERTED
    assert slider['frameSize'] == (-0.08, 0.08, -1, 1)
    assert slider['frameVisibleScale'] == (0.25, 1)
