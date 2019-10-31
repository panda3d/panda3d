from direct.gui.DirectButton import DirectButton


def test_button_destroy():
    btn = DirectButton(text="Test")
    btn.destroy()

def test_dict_get_broken():
    btn = DirectButton(text="Test", pos=(0.5, 0.5, 0.5))
    t = (0.1, 0.1, 0.1)
    btn["pos"] = t
    assert btn["pos"]!=t
    
def test_button_setPos():
    btn = DirectButton(text="Test", pos=(0.5, 0.5, 0.5))
    t = (0.1, 0.1, 0.1)
    btn["pos"] = t
    btn.setPos(*t)
    assert btn.getPos()==t
    
def test_button_setScale():
    btn = DirectButton(text="Test", pos=(0.5, 0.5, 0.5),scale=(0.5, 0.5, 0.5))
    t = (0.1, 0.1, 0.1)
    btn.setScale(*t)
    assert btn.getScale() == t
    

if __name__=="__main__":
    test_dict_get_broken()
    test_button_destroy()
    test_button_setPos()
    test_button_setScale()
    
