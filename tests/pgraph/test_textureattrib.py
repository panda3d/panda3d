from panda3d import core

# Some dummy textures we can use for our texture attributes.
stage1 = core.TextureStage("stage1")
stage2 = core.TextureStage("stage2")
stage3 = core.TextureStage("stage3")
tex1 = core.Texture("tex1")
tex2 = core.Texture("tex2")
tex3 = core.Texture("tex3")


def test_textureattrib_compose_add():
    # Tests a case in which a child node adds another texture.
    tattr1 = core.TextureAttrib.make()
    tattr1 = tattr1.add_on_stage(stage1, tex1)

    tattr2 = core.TextureAttrib.make()
    tattr2 = tattr2.add_on_stage(stage2, tex2)

    tattr3 = tattr1.compose(tattr2)
    assert tattr3.get_num_on_stages() == 2

    assert stage1 in tattr3.on_stages
    assert stage2 in tattr3.on_stages


def test_textureattrib_compose_subtract():
    # Tests a case in which a child node disables a texture.
    tattr1 = core.TextureAttrib.make()
    tattr1 = tattr1.add_on_stage(stage1, tex1)
    tattr1 = tattr1.add_on_stage(stage2, tex2)

    tattr2 = core.TextureAttrib.make()
    tattr2 = tattr2.add_off_stage(stage3)
    tattr2 = tattr2.add_off_stage(stage2)

    tattr3 = tattr1.compose(tattr2)
    assert tattr3.get_num_on_stages() == 1

    assert stage1 in tattr3.on_stages
    assert stage2 not in tattr3.on_stages
    assert stage3 not in tattr3.on_stages


def test_textureattrib_compose_both():
    # Tests a case in which a child node both enables and disables a texture.
    tattr1 = core.TextureAttrib.make()
    tattr1 = tattr1.add_on_stage(stage1, tex1)
    tattr1 = tattr1.add_on_stage(stage2, tex2)

    tattr2 = core.TextureAttrib.make()
    tattr2 = tattr2.add_on_stage(stage3, tex3)
    tattr2 = tattr2.add_on_stage(stage1, tex1)
    tattr2 = tattr2.add_off_stage(stage2)

    tattr3 = tattr1.compose(tattr2)
    assert tattr3.get_num_on_stages() == 2

    assert stage1 in tattr3.on_stages
    assert stage2 not in tattr3.on_stages
    assert stage3 in tattr3.on_stages


def test_textureattrib_compose_alloff():
    # Tests a case in which a child node disables all textures.
    tattr1 = core.TextureAttrib.make()
    tattr1 = tattr1.add_on_stage(stage1, tex1)
    tattr1 = tattr1.add_on_stage(stage2, tex2)
    assert tattr1.get_num_on_stages() == 2

    tattr2 = core.TextureAttrib.make_all_off()
    assert tattr2.has_all_off()

    tattr3 = tattr1.compose(tattr2)
    assert tattr3.get_num_on_stages() == 0
    assert tattr3.get_num_off_stages() == 0
    assert tattr3.has_all_off()


def test_textureattrib_compare():
    tattr1 = core.TextureAttrib.make()
    tattr2 = core.TextureAttrib.make()
    assert tattr1.compare_to(tattr2) == 0

    # All-off should not compare equal to empty
    tattr2 = core.TextureAttrib.make_all_off()
    assert tattr1.compare_to(tattr2) != 0
    assert tattr2.compare_to(tattr1) != 0
    assert tattr2.compare_to(tattr1) == -tattr1.compare_to(tattr2)

    # If both have the same texture, they are equal
    tattr1 = core.TextureAttrib.make()
    tattr1 = tattr1.add_on_stage(stage1, tex1)
    tattr2 = core.TextureAttrib.make()
    tattr2 = tattr2.add_on_stage(stage1, tex1)
    assert tattr1.compare_to(tattr2) == 0
    assert tattr2.compare_to(tattr1) == 0

    # Adding an extra texture makes it unequal
    tattr2 = tattr2.add_on_stage(stage2, tex2)
    assert tattr1.compare_to(tattr2) != 0
    assert tattr2.compare_to(tattr1) != 0
    assert tattr2.compare_to(tattr1) == -tattr1.compare_to(tattr2)

    # Different textures altogether is of course unequal
    tattr1 = core.TextureAttrib.make()
    tattr1 = tattr1.add_on_stage(stage2, tex2)
    tattr2 = core.TextureAttrib.make()
    tattr2 = tattr2.add_on_stage(stage1, tex1)
    assert tattr1.compare_to(tattr2) != 0
    assert tattr2.compare_to(tattr1) != 0
    assert tattr2.compare_to(tattr1) == -tattr1.compare_to(tattr2)
