from panda3d.core import WindowProperties

import pytest


def test_winprops_ctor():
    props = WindowProperties()
    assert not props.is_any_specified()


def test_winprops_copy_ctor():
    props = WindowProperties()
    props.set_size(1, 2)

    props2 = WindowProperties(props)
    assert props == props2
    assert props2.get_size() == (1, 2)

    with pytest.raises(TypeError):
        WindowProperties(None)


def test_winprops_ctor_kwargs():
    props = WindowProperties(size=(1, 2), origin=3)

    assert props.has_size()
    assert props.get_size() == (1, 2)

    assert props.has_origin()
    assert props.get_origin() == (3, 3)

    # Invalid property should throw
    with pytest.raises(TypeError):
        WindowProperties(swallow_type="african")

    # Invalid value should throw
    with pytest.raises(TypeError):
        WindowProperties(size="invalid")


def test_winprops_size_staticmethod():
    props = WindowProperties.size(1, 2)
    assert props.has_size()
    assert props.get_size() == (1, 2)

    props = WindowProperties.size((1, 2))
    assert props.has_size()
    assert props.get_size() == (1, 2)


def test_winprops_size_property():
    props = WindowProperties()

    # Test get
    props.set_size(1, 2)
    assert props.size == (1, 2)

    # Test has
    props.clear_size()
    assert props.size is None

    # Test set
    props.size = (4, 5)
    assert props.get_size() == (4, 5)

    # Test clear
    props.size = None
    assert not props.has_size()
