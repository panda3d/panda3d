from panda3d.core import TextNode
from direct.gui.DirectButton import DirectButton


def test_button_default_extraArgs():
    btn = DirectButton()

    assert btn.configure('extraArgs') == ('extraArgs', [], [])
    assert btn._optionInfo['extraArgs'] == [[], [], None]

    # Changing will not affect default value
    btn['extraArgs'].append(1)
    assert btn.configure('extraArgs') == ('extraArgs', [], [1])

    # Changing this does
    btn.configure('extraArgs')[1].append(2)
    assert btn.configure('extraArgs') == ('extraArgs', [2], [1])


def test_button_destroy():
    btn = DirectButton(text="Test")
    btn.destroy()


def test_button_text_align_cget():
    btn = DirectButton(text="Kitten", text_align=TextNode.ALeft, scale=0.07)
    try:
        assert btn["text_align"] == TextNode.ALeft
        btn["text_align"] = TextNode.ACenter
        assert btn["text_align"] == TextNode.ACenter
        btn["text_align"] = TextNode.ARight
        assert btn["text_align"] == TextNode.ARight
    finally:
        btn.destroy()
