from direct.gui.OnscreenText import OnscreenText
import pytest


@pytest.mark.filterwarnings("ignore::DeprecationWarning")
def test_onscreentext_text_pos():
    text = OnscreenText(pos=(1, 2))
    assert text['pos'] == (1, 2)
    assert text.pos == (1, 2)
    assert text.getPos() == (1, 2)
    assert text.text_pos == (1, 2)
    assert text.getTextPos() == (1, 2)
    assert text.get_pos() == (0, 0, 0)

    text.setTextPos(3, 4)
    assert text['pos'] == (3, 4)
    assert text.pos == (3, 4)
    assert text.getPos() == (3, 4)
    assert text.text_pos == (3, 4)
    assert text.getTextPos() == (3, 4)
    assert text.get_pos() == (0, 0, 0)

    text.text_pos = (7, 8)
    assert text['pos'] == (7, 8)
    assert text.pos == (7, 8)
    assert text.getPos() == (7, 8)
    assert text.text_pos == (7, 8)
    assert text.getTextPos() == (7, 8)
    assert text.get_pos() == (0, 0, 0)

    text.setPos(9, 10)
    assert text['pos'] == (9, 10)
    assert text.pos == (9, 10)
    assert text.getPos() == (9, 10)
    assert text.text_pos == (9, 10)
    assert text.getTextPos() == (9, 10)
    assert text.get_pos() == (0, 0, 0)

    text['pos'] = (11, 12)
    assert text['pos'] == (11, 12)
    assert text.pos == (11, 12)
    assert text.getPos() == (11, 12)
    assert text.text_pos == (11, 12)
    assert text.getTextPos() == (11, 12)
    assert text.get_pos() == (0, 0, 0)


@pytest.mark.filterwarnings("ignore::DeprecationWarning")
def test_onscreentext_node_pos():
    text = OnscreenText()

    text.set_pos(1, 2, 3)
    assert text['pos'] == (0, 0)
    assert text.pos == (0, 0)
    assert text.getPos() == (0, 0)
    assert text.text_pos == (0, 0)
    assert text.getTextPos() == (0, 0)
    assert text.get_pos() == (1, 2, 3)


@pytest.mark.filterwarnings("ignore::DeprecationWarning")
def test_onscreentext_text_roll():
    text = OnscreenText(roll=1)
    assert text['roll'] == 1
    assert text.roll == 1
    assert text.getRoll() == 1
    assert text.text_r == -1
    assert text.getTextR() == -1
    assert text.get_r() == 0

    text.setTextR(2)
    assert text['roll'] == -2
    assert text.roll == -2
    assert text.getRoll() == -2
    assert text.text_r == 2
    assert text.getTextR() == 2
    assert text.get_r() == 0

    text.text_r = 3
    assert text['roll'] == -3
    assert text.roll == -3
    assert text.getRoll() == -3
    assert text.text_r == 3
    assert text.getTextR() == 3
    assert text.get_r() == 0

    text.setRoll(4)
    assert text['roll'] == 4
    assert text.roll == 4
    assert text.getRoll() == 4
    assert text.text_r == -4
    assert text.getTextR() == -4
    assert text.get_r() == 0

    text['roll'] = 5
    assert text['roll'] == 5
    assert text.roll == 5
    assert text.getRoll() == 5
    assert text.text_r == -5
    assert text.getTextR() == -5
    assert text.get_r() == 0


@pytest.mark.filterwarnings("ignore::DeprecationWarning")
def test_onscreentext_node_roll():
    text = OnscreenText()

    text.set_r(45)
    assert text['roll'] == 0
    assert text.roll == 0
    assert text.getRoll() == 0
    assert text.text_r == 0
    assert text.getTextR() == 0
    assert text.get_r() == 45


@pytest.mark.filterwarnings("ignore::DeprecationWarning")
def test_onscreentext_text_scale():
    text = OnscreenText(scale=(1, 2))
    assert text['scale'] == (1, 2)
    assert text.scale == (1, 2)
    assert text.getScale() == (1, 2)
    assert text.text_scale == (1, 2)
    assert text.getTextScale() == (1, 2)
    assert text.get_scale() == (1, 1, 1)

    text.setTextScale(3, 4)
    assert text['scale'] == (3, 4)
    assert text.scale == (3, 4)
    assert text.getScale() == (3, 4)
    assert text.text_scale == (3, 4)
    assert text.getTextScale() == (3, 4)
    assert text.get_scale() == (1, 1, 1)

    text.text_scale = (7, 8)
    assert text['scale'] == (7, 8)
    assert text.scale == (7, 8)
    assert text.getScale() == (7, 8)
    assert text.text_scale == (7, 8)
    assert text.getTextScale() == (7, 8)
    assert text.get_scale() == (1, 1, 1)

    text.setScale(9, 10)
    assert text['scale'] == (9, 10)
    assert text.scale == (9, 10)
    assert text.getScale() == (9, 10)
    assert text.text_scale == (9, 10)
    assert text.getTextScale() == (9, 10)
    assert text.get_scale() == (1, 1, 1)

    text['scale'] = (11, 12)
    assert text['scale'] == (11, 12)
    assert text.scale == (11, 12)
    assert text.getScale() == (11, 12)
    assert text.text_scale == (11, 12)
    assert text.getTextScale() == (11, 12)
    assert text.get_scale() == (1, 1, 1)

    text.scale = 13
    assert text['scale'] == (13, 13)
    assert text.scale == (13, 13)
    assert text.getScale() == (13, 13)
    assert text.text_scale == (13, 13)
    assert text.getTextScale() == (13, 13)
    assert text.get_scale() == (1, 1, 1)


@pytest.mark.filterwarnings("ignore::DeprecationWarning")
def test_onscreentext_node_scale():
    text = OnscreenText()

    text.set_scale(1, 2, 3)
    assert text['scale'] == (0.07, 0.07)
    assert text.scale == (0.07, 0.07)
    assert text.getScale() == (0.07, 0.07)
    assert text.text_scale == (0.07, 0.07)
    assert text.getTextScale() == (0.07, 0.07)
    assert text.get_scale() == (1, 2, 3)
