import os
import platform
import pytest
from _pytest.outcomes import Failed

from panda3d import core


SHADERS_DIR = core.Filename.from_os_specific(os.path.dirname(__file__))


def run_cg_compile_check(shader_path, expect_fail=False):
    """Compile supplied Cg shader path and check for errors"""
    shader = core.Shader.load(shader_path, core.Shader.SL_Cg)
    # assert shader.is_prepared(gsg.prepared_objects)
    if expect_fail:
        assert shader is None
    else:
        assert shader is not None


def test_cg_compile_error():
    """Test getting compile errors from bad Cg shaders"""
    shader_path = core.Filename(SHADERS_DIR, 'cg_bad.sha')
    run_cg_compile_check(shader_path, expect_fail=True)


def test_cg_from_file():
    """Test compiling Cg shaders from files"""
    shader_path = core.Filename(SHADERS_DIR, 'cg_simple.sha')
    run_cg_compile_check(shader_path)


def test_cg_test(env):
    "Test to make sure that the Cg tests work correctly."

    env.run_cg("assert(true);")


def test_cg_test_fail(env):
    "Same as above, but making sure that the failure case works correctly."

    with pytest.raises(Failed):
        env.run_cg("assert(false);")


def test_cg_sampler(env):
    tex1 = core.Texture("tex1-ubyte-rgba8")
    tex1.setup_1d_texture(1, core.Texture.T_unsigned_byte, core.Texture.F_rgba8)
    tex1.set_clear_color((0, 2 / 255.0, 1, 1))

    tex2 = core.Texture("tex2-float-rgba32")
    tex2.setup_2d_texture(1, 1, core.Texture.T_float, core.Texture.F_rgba32)
    tex2.set_clear_color((1.0, 2.0, -3.14, 0.0))

    tex3 = core.Texture("tex3-float-r32")
    tex3.setup_3d_texture(1, 1, 1, core.Texture.T_float, core.Texture.F_r32)
    tex3.set_clear_color((0.5, 0.0, 0.0, 1.0))

    preamble = """
    uniform sampler1D tex1;
    uniform sampler2D tex2;
    uniform sampler3D tex3;
    """
    code = """
    assert(tex1D(tex1, 0) == float4(0, 2 / 255.0, 1, 1));
    assert(tex2D(tex2, float2(0, 0)) == float4(1.0, 2.0, -3.14, 0.0));
    assert(abs(tex3D(tex3, float3(0, 0, 0)).r - 0.5) < 0.01);
    """
    env.run_cg(code, preamble, {'tex1': tex1, 'tex2': tex2, 'tex3': tex3})


def test_cg_int(env):
    inputs = dict(
        zero=0,
        ten=10,
        intmax=0x7fffffff,
        intmin=-0x7fffffff,
    )
    preamble = """
    uniform int zero;
    uniform int intmax;
    uniform int intmin;
    """
    code = """
    assert(zero == 0);
    assert(intmax == 0x7fffffff);
    assert(intmin == -0x7fffffff);
    """
    env.run_cg(code, preamble, inputs)


def test_cg_state_material(env):
    mat = core.Material("mat")
    mat.ambient = (1, 2, 3, 4)
    mat.diffuse = (5, 6, 7, 8)
    mat.emission = (9, 10, 11, 12)
    mat.specular = (13, 14, 15, 0)
    mat.shininess = 16

    preamble = """
    uniform float4x4 attr_material;
    """
    code = """
    assert(attr_material[0] == float4(1, 2, 3, 4));
    assert(attr_material[1] == float4(5, 6, 7, 8));
    assert(attr_material[2] == float4(9, 10, 11, 12));
    assert(attr_material[3].rgb == float3(13, 14, 15));
    assert(attr_material[3].w == 16);
    """

    node = core.NodePath("state")
    node.set_material(mat)

    env.run_cg(code, preamble, state=node.get_state())


def test_cg_state_fog(env):
    fog = core.Fog("fog")
    fog.color = (1, 2, 3, 4)
    fog.exp_density = 0.5
    fog.set_linear_range(6, 10)

    preamble = """
    uniform float4 attr_fog;
    uniform float4 attr_fogcolor;
    """
    code = """
    assert(attr_fogcolor == float4(1, 2, 3, 4));
    assert(attr_fog[0] == 0.5);
    assert(attr_fog[1] == 6);
    assert(attr_fog[2] == 10);
    assert(attr_fog[3] == 0.25);
    """

    node = core.NodePath("state")
    node.set_fog(fog)

    env.run_cg(code, preamble, state=node.get_state())


def test_cg_texpad_texpix(env):
    tex = core.Texture("test")
    tex.setup_2d_texture(16, 32, core.Texture.T_unsigned_byte, core.Texture.F_rgba)
    tex.auto_texture_scale = core.ATS_pad
    tex.set_size_padded(10, 30)

    preamble = """
    uniform float3 texpad_test;
    uniform float2 texpix_test;
    """
    code = """
    assert(texpad_test == float3(10 * 0.5 / 16, 30 * 0.5 / 32, 0.5));
    assert(texpix_test == float2(1.0 / 16, 1.0 / 32));
    """

    env.run_cg(code, preamble, inputs={"test": tex})


def test_cg_alight(env):
    alight = core.AmbientLight("alight")
    alight.set_color((1, 2, 3, 4))
    np = core.NodePath(alight)

    preamble = """
    uniform float4 alight_test;
    """
    code = """
    assert(alight_test == float4(1, 2, 3, 4));
    """

    env.run_cg(code, preamble, inputs={"test": np})


def test_cg_satten(env):
    spot = core.Spotlight("spot")
    spot.set_attenuation((1, 2, 3))
    spot.set_exponent(4)
    np = core.NodePath(spot)

    preamble = """
    uniform float4 satten_test;
    """
    code = """
    assert(satten_test == float4(1, 2, 3, 4));
    """

    env.run_cg(code, preamble, inputs={"test": np})
