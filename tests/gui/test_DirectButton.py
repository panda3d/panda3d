from direct.gui.DirectButton import DirectButton


def test_button_destroy():
    btn = DirectButton(text="Test")
    btn.destroy()
