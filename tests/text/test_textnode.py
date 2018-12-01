from panda3d import core


def test_textnode_write():
    out = core.StringStream()
    text = core.TextNode("test")
    text.write(out, 0)
    assert out.data.startswith(b"TextNode test")


def test_textnode_card_as_margin():
    text = core.TextNode("test")
    text.text = "Test"

    l, r, b, t = 0.1, 0.2, 0.3, 0.4
    text.set_card_as_margin(l, r, b, t)

    assert text.has_card()
    assert text.is_card_as_margin()
    assert text.get_card_as_set() == (l, r, b, t)

    card_actual = text.get_card_actual()
    card_expect = core.LVecBase4(
        text.get_left() - l,
        text.get_right() + r,
        text.get_bottom() - b,
        text.get_top() + t)
    assert card_actual == card_expect


def test_textnode_card_actual():
    text = core.TextNode("test")
    text.text = "Test"

    l, r, b, t = 0.1, 0.2, 0.3, 0.4
    text.set_card_actual(l, r, b, t)

    assert text.has_card()
    assert not text.is_card_as_margin()
    assert text.get_card_as_set() == (l, r, b, t)

    card_actual = text.get_card_actual()
    card_expect = core.LVecBase4(l, r, b, t)
    assert card_actual == card_expect


def test_textnode_frame_as_margin():
    text = core.TextNode("test")
    text.text = "Test"

    l, r, b, t = 0.1, 0.2, 0.3, 0.4
    text.set_frame_as_margin(l, r, b, t)

    assert text.has_frame()
    assert text.is_frame_as_margin()
    assert text.get_frame_as_set() == (l, r, b, t)

    frame_actual = text.get_frame_actual()
    frame_expect = core.LVecBase4(
        text.get_left() - l,
        text.get_right() + r,
        text.get_bottom() - b,
        text.get_top() + t)
    assert frame_actual == frame_expect


def test_textnode_frame_actual():
    text = core.TextNode("test")
    text.text = "Test"

    l, r, b, t = 0.1, 0.2, 0.3, 0.4
    text.set_frame_actual(l, r, b, t)

    assert text.has_frame()
    assert not text.is_frame_as_margin()
    assert text.get_frame_as_set() == (l, r, b, t)

    frame_actual = text.get_frame_actual()
    frame_expect = core.LVecBase4(l, r, b, t)
    assert frame_actual == frame_expect


def test_textnode_flatten_color():
    text = core.TextNode("test")
    text.text_color = (0, 0, 0, 1)
    path = core.NodePath(text)

    color = core.LColor(1, 0, 0, 1)
    path.set_color(color)
    path.flatten_strong()

    assert text.text_color.almost_equal(color)
    assert text.shadow_color.almost_equal(color)
    assert text.frame_color.almost_equal(color)
    assert text.card_color.almost_equal(color)


def test_textnode_flatten_colorscale():
    text = core.TextNode("test")
    text.text_color = (1, 0, 0, 0)
    text.shadow_color = (0, 1, 0, 0)
    text.frame_color = (0, 0, 1, 0)
    text.card_color = (0, 0, 0, 1)
    path = core.NodePath(text)

    color = core.LColor(.5, .5, .5, .5)
    path.set_color_scale(color)
    path.flatten_strong()

    assert text.text_color.almost_equal((.5, 0, 0, 0))
    assert text.shadow_color.almost_equal((0, .5, 0, 0))
    assert text.frame_color.almost_equal((0, 0, .5, 0))
    assert text.card_color.almost_equal((0, 0, 0, .5))
