from panda3d import core


def test_shaderattrib_flags():
    # Ensure the old single-flag behavior still works

    shattr = core.ShaderAttrib.make()

    # Make sure we have the flag
    shattr = shattr.set_flag(core.ShaderAttrib.F_hardware_skinning, True)
    assert shattr.get_flag(core.ShaderAttrib.F_hardware_skinning)

    # Make sure we don't have a flag that we didn't set
    assert not shattr.get_flag(core.ShaderAttrib.F_subsume_alpha_test)

    # Clear it, we should not longer have the flag
    shattr = shattr.clear_flag(core.ShaderAttrib.F_hardware_skinning)
    assert not shattr.get_flag(core.ShaderAttrib.F_hardware_skinning)

    # Set a flag to false, we shouldn't have it
    shattr = shattr.set_flag(core.ShaderAttrib.F_hardware_skinning, False)
    assert not shattr.get_flag(core.ShaderAttrib.F_hardware_skinning)

    # Ensure the new behavior works
    shattr = core.ShaderAttrib.make()

    # Make sure we have the flags
    shattr = shattr.set_flag(core.ShaderAttrib.F_hardware_skinning | core.ShaderAttrib.F_subsume_alpha_test, True)
    assert shattr.get_flag(core.ShaderAttrib.F_hardware_skinning | core.ShaderAttrib.F_subsume_alpha_test)

    # Make sure we don't have a flag that we didn't set
    assert not shattr.get_flag(core.ShaderAttrib.F_shader_point_size)
    # ...group of flags we didn't set
    assert not shattr.get_flag(core.ShaderAttrib.F_disable_alpha_write | core.ShaderAttrib.F_shader_point_size)

    # Make sure they clear correctly
    shattr = shattr.clear_flag(core.ShaderAttrib.F_hardware_skinning | core.ShaderAttrib.F_subsume_alpha_test)
    assert not shattr.get_flag(core.ShaderAttrib.F_hardware_skinning | core.ShaderAttrib.F_subsume_alpha_test)

    # Set group to false
    shattr = shattr.set_flag(core.ShaderAttrib.F_hardware_skinning | core.ShaderAttrib.F_subsume_alpha_test, False)
    assert not shattr.get_flag(core.ShaderAttrib.F_hardware_skinning | core.ShaderAttrib.F_subsume_alpha_test)
