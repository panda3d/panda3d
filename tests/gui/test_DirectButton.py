from direct.gui.DirectButton import DirectButton


def test_button_destroy():
    btn = DirectButton(text="Test")
    btn.destroy()

def test_button_setPos():
    btn = DirectButton(text="Test", pos=(0.5, 0.5, 0.5))
    
    btn["pos"] = (0.1, 0.1, 0.1)
    assert btn["pos"] == (0.1, 0.1, 0.1)
    

if __name__=="__main__":
    test_button_destroy()
    test_button_setPos()
    
