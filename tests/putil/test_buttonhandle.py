from panda3d.core import ButtonHandle
from panda3d.core import GamepadButton
from panda3d.core import KeyboardButton
from panda3d.core import MouseButton


def test_buttonhandle_type():
    assert ButtonHandle.get_class_type().name == "ButtonHandle"


def test_buttonhandle_none():
    none = ButtonHandle.none()
    assert none.index == 0
    assert none.name == "none"
    assert none == ButtonHandle.none()
    assert none.alias == none
    assert repr(none) == "none"
    assert str(none) == "none"


def test_gamepadbutton_joystick():
    # The first one is called "trigger"
    assert GamepadButton.trigger() == GamepadButton.joystick(0)
    assert GamepadButton.joystick(0).name == "trigger"

    for i in range(1, 8):
        btn = GamepadButton.joystick(i)
        assert btn.name == "joystick" + str(i + 1)


def test_keyboardbutton_ascii():
    assert KeyboardButton.space() == KeyboardButton.ascii_key(' ')
    assert KeyboardButton.backspace() == KeyboardButton.ascii_key('\x08')
    assert KeyboardButton.tab() == KeyboardButton.ascii_key('\x09')
    assert KeyboardButton.enter() == KeyboardButton.ascii_key('\x0d')
    assert KeyboardButton.escape() == KeyboardButton.ascii_key('\x1b')

    assert KeyboardButton.ascii_key(' ').name == 'space'
    assert KeyboardButton.ascii_key('\x08').name == 'backspace'
    assert KeyboardButton.ascii_key('\x09').name == 'tab'
    assert KeyboardButton.ascii_key('\x0d').name == 'enter'
    assert KeyboardButton.ascii_key('\x1b').name == 'escape'
    assert KeyboardButton.ascii_key('\x7f').name == 'delete'

    assert KeyboardButton.ascii_key('a').name == 'a'


def test_mousebutton():
    btns = [MouseButton.one(),
            MouseButton.two(),
            MouseButton.three(),
            MouseButton.four(),
            MouseButton.five()]

    for i, btn in enumerate(btns):
        assert MouseButton.button(i) == btn
        assert MouseButton.is_mouse_button(btn)

    assert MouseButton.button(5) == ButtonHandle.none()

    assert MouseButton.is_mouse_button(MouseButton.wheel_up())
    assert MouseButton.is_mouse_button(MouseButton.wheel_down())
    assert MouseButton.is_mouse_button(MouseButton.wheel_left())
    assert MouseButton.is_mouse_button(MouseButton.wheel_right())
