from direct.gui.DirectFrame import DirectFrame
from panda3d.core import NodePath, Texture


def test_frame_empty():
    frame = DirectFrame()
    assert not frame.hascomponent('text0')
    assert not frame.hascomponent('geom0')
    assert not frame.hascomponent('image0')


def test_frame_text():
    frame = DirectFrame(text="Test")
    assert frame.hascomponent('text0')
    assert not frame.hascomponent('text1')
    assert frame.component('text0').text == "Test"

    # Change text
    frame.setText("Foo")
    assert frame.component('text0').text == "Foo"

    # Change text to unicode
    frame.setText(u"Foo")

    # Clear text
    frame.clearText()
    assert not frame.hascomponent('text0')


def test_frame_text_states():
    frame = DirectFrame(text=("A", "B", "C"), numStates=3)
    assert frame.hascomponent('text0')
    assert frame.hascomponent('text1')
    assert frame.hascomponent('text2')
    assert not frame.hascomponent('text3')

    assert frame.component('text0').text == "A"
    assert frame.component('text1').text == "B"
    assert frame.component('text2').text == "C"

    # Change text for all states
    frame.setText("Foo")

    assert frame.component('text0').text == "Foo"
    assert frame.component('text1').text == "Foo"
    assert frame.component('text2').text == "Foo"

    # Change text per state
    frame.setText(("1", "2", "3"))
    assert frame.component('text0').text == "1"
    assert frame.component('text1').text == "2"
    assert frame.component('text2').text == "3"

    # Changing via list should work too
    frame.setText(["1", "2", "3"])

    # Clear text
    frame.clearText()
    assert not frame.hascomponent('text0')
    assert not frame.hascomponent('text1')
    assert not frame.hascomponent('text2')


def test_frame_geom():
    frame = DirectFrame(geom=NodePath("geom-a"))
    assert frame.hascomponent('geom0')
    assert not frame.hascomponent('geom1')
    assert frame.component('geom0').name == "geom-a"

    # Change geom
    frame.setGeom(NodePath("geom-b"))
    assert frame.component('geom0').name == "geom-b"

    # Clear geom
    frame.clearGeom()
    assert not frame.hascomponent('geom0')


def test_frame_geom_states():
    frame = DirectFrame(geom=(NodePath("A"), NodePath("B"), NodePath("C")), numStates=3)
    assert frame.hascomponent('geom0')
    assert frame.hascomponent('geom1')
    assert frame.hascomponent('geom2')
    assert not frame.hascomponent('geom3')

    assert frame.component('geom0').name == "A"
    assert frame.component('geom1').name == "B"
    assert frame.component('geom2').name == "C"

    # Change geom for all states
    frame.setGeom(NodePath("Foo"))

    assert frame.component('geom0').name == "Foo"
    assert frame.component('geom1').name == "Foo"
    assert frame.component('geom2').name == "Foo"

    # Change geom per state
    states = (NodePath("1"), NodePath("2"), NodePath("3"))
    frame.setGeom(states)
    assert frame.component('geom0').name == "1"
    assert frame.component('geom1').name == "2"
    assert frame.component('geom2').name == "3"

    # Changing via list should work too
    frame.setGeom(list(states))

    # Clear geom
    frame.clearGeom()
    assert not frame.hascomponent('geom0')
    assert not frame.hascomponent('geom1')
    assert not frame.hascomponent('geom2')
