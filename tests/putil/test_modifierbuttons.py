from panda3d.core import ModifierButtons


def test_modifierbuttons_empty():
    # Tests the initial state of a ModifierButtons object.
    btns = ModifierButtons()
    assert btns == ModifierButtons(btns)
    assert btns != ModifierButtons()
    assert btns.matches(ModifierButtons())
    assert not btns.is_down("alt")
    assert not btns.is_any_down()
    assert not btns.has_button("alt")
    assert btns.get_prefix() == ""
    assert btns.get_num_buttons() == 0
    assert len(btns.buttons) == 0


def test_modifierbuttons_cow():
    # Tests the copy-on-write mechanism of the button list.
    btns1 = ModifierButtons()
    btns1.add_button("space")

    # Modifying original should not affect copy
    btns2 = ModifierButtons(btns1)
    assert tuple(btns2.buttons) == tuple(btns1.buttons)
    btns1.add_button("enter")
    assert tuple(btns1.buttons) == ("space", "enter")
    assert tuple(btns2.buttons) == ("space",)

    # Modifying copy should not affect original
    btns3 = ModifierButtons(btns2)
    assert tuple(btns3.buttons) == tuple(btns2.buttons)
    btns3.add_button("escape")
    assert tuple(btns2.buttons) == ("space",)
    assert tuple(btns3.buttons) == ("space", "escape")
