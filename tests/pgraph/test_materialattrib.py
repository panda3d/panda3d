from panda3d import core


def test_materialattrib_compare():
    mat1 = core.Material()
    mat2 = core.Material()

    # Two empty attribs
    mattr1 = core.MaterialAttrib.make_off()
    mattr2 = core.MaterialAttrib.make_off()
    assert mattr1.compare_to(mattr2) == 0
    assert mattr2.compare_to(mattr1) == 0

    # One empty attrib, one with a material
    mattr1 = core.MaterialAttrib.make_off()
    mattr2 = core.MaterialAttrib.make(mat1)
    assert mattr1 != mattr2
    assert mattr1.compare_to(mattr2) != 0
    assert mattr2.compare_to(mattr1) != 0
    assert mattr1.compare_to(mattr2) == -mattr2.compare_to(mattr1)

    # Two attribs with same material
    mattr1 = core.MaterialAttrib.make(mat1)
    mattr2 = core.MaterialAttrib.make(mat1)
    assert mattr1.compare_to(mattr2) == 0
    assert mattr2.compare_to(mattr1) == 0

    # Two different materials
    mattr1 = core.MaterialAttrib.make(mat1)
    mattr2 = core.MaterialAttrib.make(mat2)
    assert mattr1 != mattr2
    assert mattr1.compare_to(mattr2) != 0
    assert mattr2.compare_to(mattr1) != 0
    assert mattr1.compare_to(mattr2) == -mattr2.compare_to(mattr1)
