def test_window_basic(window):
    from panda3d.core import WindowProperties
    assert window is not None

    current_props = window.get_properties()
    default_props = WindowProperties.get_default()

    # Opening the window changes these from the defaults.  Note that we have
    # no guarantee that it opens in the foreground or with the requested size.
    default_props.set_size(current_props.get_size())
    default_props.set_origin(current_props.get_origin())
    default_props.set_minimized(False)
    default_props.foreground = current_props.foreground
    default_props.z_order = current_props.z_order

    # The rest should be the same
    assert current_props == default_props
