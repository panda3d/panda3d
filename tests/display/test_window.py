def test_window_basic(window):
    from panda3d.core import WindowProperties
    assert window is not None

    current_props = window.get_properties()
    default_props = WindowProperties.get_default()

    # Opening the window changes these from the defaults
    default_props.set_size(current_props.get_size())
    default_props.set_origin(current_props.get_origin())
    default_props.set_minimized(False)
    default_props.set_foreground(True)

    # The rest should be the same
    assert current_props == default_props
