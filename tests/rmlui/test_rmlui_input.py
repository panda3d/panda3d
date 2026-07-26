import pytest

rmlui = pytest.importorskip("panda3d.rmlui")
from panda3d import core


def test_input_handler_construction():
    handler = rmlui.RmlInputHandler()
    assert handler is not None
    # It is a DataNode.
    assert isinstance(handler, core.DataNode)


def test_get_rml_key_known_keys():
    # Letters and digits map to non-zero RmlUi key identifiers.
    a = rmlui.RmlInputHandler.get_rml_key(core.KeyboardButton.ascii_key('a'))
    z = rmlui.RmlInputHandler.get_rml_key(core.KeyboardButton.ascii_key('z'))
    zero = rmlui.RmlInputHandler.get_rml_key(core.KeyboardButton.ascii_key('0'))
    assert a != 0
    assert z != 0
    assert zero != 0
    assert a != z


def test_get_rml_key_special_keys():
    for btn in (core.KeyboardButton.space(),
                core.KeyboardButton.enter(),
                core.KeyboardButton.escape(),
                core.KeyboardButton.tab(),
                core.KeyboardButton.left(),
                core.KeyboardButton.f1()):
        assert rmlui.RmlInputHandler.get_rml_key(btn) != 0


def test_get_rml_key_unmapped_returns_zero():
    # A button with no RmlUi equivalent maps to KI_UNKNOWN (0).
    assert rmlui.RmlInputHandler.get_rml_key(core.MouseButton.one()) == 0
