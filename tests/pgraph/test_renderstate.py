from panda3d.core import RenderState, TransparencyAttrib, ColorAttrib
import pytest


def test_renderstate_make():
    assert RenderState.make() == RenderState.make_empty()
    assert RenderState.make(override=123) == RenderState.make_empty()

    with pytest.raises(TypeError):
        RenderState.make(override=0, blargh=123)
        RenderState.make(blargh=123)

    with pytest.raises(OverflowError):
        RenderState.make(override=0x80000000)
        RenderState.make(override=-0x80000000)

    state = RenderState.make(ColorAttrib.make_vertex(), TransparencyAttrib.make_default())
    assert state.has_attrib(ColorAttrib)
    assert state.has_attrib(TransparencyAttrib)
    assert state.attribs[ColorAttrib] == ColorAttrib.make_vertex()
    assert state.attribs[TransparencyAttrib] == TransparencyAttrib.make_default()
