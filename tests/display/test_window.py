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

    # The rest should be the same
    assert current_props == default_props


def test_window_prop_copy(window, win_props):
    assert win_props == window.properties


def change_window_properties(window, new_props):
    """Request changes on a window and force the window to update"""
    from panda3d.core import WindowProperties

    window.request_properties(new_props)
    window.engine.render_frame()

    # Clear any rejected props we no will get rejected then check if any are
    # left
    rejected_props = WindowProperties(window.rejected_properties)
    rejected_props.clear_fixed_size()
    print("Rejected Properties", rejected_props)

    assert not rejected_props.is_any_specified()


def test_window_cursor_hidden(window, win_props):
    win_props.set_cursor_hidden(True)
    change_window_properties(window, win_props)
    assert win_props == window.properties

    win_props.set_cursor_hidden(False)
    change_window_properties(window, win_props)
    assert win_props == window.properties


def test_window_foreground(window, win_props):
    win_props.set_foreground(True)
    change_window_properties(window, win_props)
    assert win_props == window.properties

    win_props.set_foreground(False)
    change_window_properties(window, win_props)
    assert win_props == window.properties


def test_window_fullscreen(window, win_props):
    win_props.set_fullscreen(True)
    change_window_properties(window, win_props)
    assert win_props.fullscreen == window.properties.fullscreen

    win_props.set_fullscreen(False)
    change_window_properties(window, win_props)
    assert win_props.fullscreen == window.properties.fullscreen


def test_window_minimized(window, win_props):
    win_props.set_minimized(True)
    change_window_properties(window, win_props)
    assert win_props.minimized == window.properties.minimized

    win_props.set_minimized(False)
    change_window_properties(window, win_props)
    assert win_props.minimized == window.properties.minimized


def test_window_mouse_mode(window, win_props):
    from panda3d.core import WindowProperties

    win_props.set_mouse_mode(WindowProperties.M_absolute)
    change_window_properties(window, win_props)
    assert win_props.mouse_mode == window.properties.mouse_mode

    win_props.set_mouse_mode(WindowProperties.M_confined)
    change_window_properties(window, win_props)
    assert win_props.mouse_mode == window.properties.mouse_mode

    win_props.set_mouse_mode(WindowProperties.M_relative)
    change_window_properties(window, win_props)
    assert win_props.mouse_mode == window.properties.mouse_mode


def test_window_origin(window, win_props):
    win_props.set_origin(1, 1)

    assert win_props.origin != window.properties.origin
    change_window_properties(window, win_props)
    assert win_props.origin == window.properties.origin


def test_window_size(window, win_props):
    win_props.set_size(1024, 768)

    assert win_props.get_size() != window.properties.get_size()
    change_window_properties(window, win_props)
    assert win_props.get_size() == window.properties.get_size()


def test_window_title(window, win_props):
    win_props.set_title("New Title")

    assert win_props.title != window.properties.title
    change_window_properties(window, win_props)
    assert win_props.title == window.properties.title


def test_window_undecorated(window, win_props):
    win_props.set_undecorated(True)
    change_window_properties(window, win_props)
    assert win_props.undecorated == window.properties.undecorated

    win_props.set_undecorated(False)
    change_window_properties(window, win_props)
    assert win_props.undecorated == window.properties.undecorated


def test_window_z_order(window, win_props):
    from panda3d.core import WindowProperties

    win_props.set_z_order(WindowProperties.Z_bottom)
    change_window_properties(window, win_props)
    assert win_props.z_order == window.properties.z_order

    win_props.set_z_order(WindowProperties.Z_normal)
    change_window_properties(window, win_props)
    assert win_props.z_order == window.properties.z_order

    win_props.set_z_order(WindowProperties.Z_top)
    change_window_properties(window, win_props)
    assert win_props.z_order == window.properties.z_order
