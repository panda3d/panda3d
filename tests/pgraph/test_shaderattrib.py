from panda3d import core


def test_shaderattrib_compare():
    shattr1 = core.ShaderAttrib.make()
    shattr2 = core.ShaderAttrib.make()
    assert shattr1.compare_to(shattr2) == 0
    assert shattr2.compare_to(shattr1) == 0

    shattr2 = core.ShaderAttrib.make().set_flag(core.ShaderAttrib.F_subsume_alpha_test, False)
    assert shattr1.compare_to(shattr2) != 0
    assert shattr2.compare_to(shattr1) != 0

    shattr1 = core.ShaderAttrib.make().set_flag(core.ShaderAttrib.F_subsume_alpha_test, False)
    assert shattr1.compare_to(shattr2) == 0
    assert shattr2.compare_to(shattr1) == 0

    shattr2 = core.ShaderAttrib.make().set_flag(core.ShaderAttrib.F_subsume_alpha_test, True)
    assert shattr1.compare_to(shattr2) != 0
    assert shattr2.compare_to(shattr1) != 0
