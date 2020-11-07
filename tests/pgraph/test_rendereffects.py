from panda3d import core


def test_rendereffects_compare():
    re1 = core.RenderEffects.make_empty()
    re2 = core.RenderEffects.make_empty()

    assert re1 == re1
    assert not (re1 != re1)
    assert not (re1 < re1)
    assert not (re1 > re1)

    assert re1 == re2
    assert not (re1 != re2)
    assert not (re1 < re2)
    assert not (re1 > re2)

    assert re1 != 123

    rd = core.RenderEffects.make(core.DecalEffect.make())
    assert not (re1 == rd)
    assert not (rd == re1)
    assert re1 != rd
    assert rd != re1
    assert re1 < rd or rd < re1
    assert re1 > rd or rd > re1
