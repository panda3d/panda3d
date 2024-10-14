import os
import platform
import pytest
from _pytest.outcomes import Failed

from panda3d import core


SHADERS_DIR = core.Filename.from_os_specific(os.path.dirname(__file__))


# This is the template for the shader that is used by run_cg_test.
# We render this to an nx1 texture, where n is the number of lines in the body.
# An assert
CG_VERTEX_TEMPLATE = """//Cg

void vshader(float4 vtx_position : POSITION, out float4 l_position : POSITION) {{
    l_position = vtx_position;
}}
"""

CG_FRAGMENT_TEMPLATE = """//Cg

{preamble}

float4 _assert(bool cond) {{
    return float4(cond.x, 1, 1, 1);
}}

float4 _assert(bool2 cond) {{
    return float4(cond.x, cond.y, 1, 1);
}}

float4 _assert(bool3 cond) {{
    return float4(cond.x, cond.y, cond.z, 1);
}}

float4 _assert(bool4 cond) {{
    return float4(cond.x, cond.y, cond.z, cond.w);
}}

#define assert(cond) {{ if ((int)l_vpos.x == __LINE__ - line_offset) o_color = _assert(cond); }}

void fshader(in float2 l_vpos : VPOS, out float4 o_color : COLOR) {{
    o_color = float4(1, 1, 1, 1);

    if ((int)l_vpos.x == 0) {{
        o_color = float4(0, 0, 0, 0);
    }}
    const int line_offset = __LINE__;
{body}
}}
"""


def run_cg_test(gsg, body, preamble="", inputs={},
                state=core.RenderState.make_empty()):
    """ Runs a Cg test on the given GSG.  The given body is executed in the
    main function and should call assert().  The preamble should contain all
    of the shader inputs. """

    if not gsg.supports_basic_shaders:
        pytest.skip("basic shaders not supported")

    __tracebackhide__ = True

    preamble = preamble.strip()
    body = body.rstrip().lstrip('\n')
    num_lines = body.count('\n') + 1

    vertex_code = CG_VERTEX_TEMPLATE.format(preamble=preamble, body=body)
    code = CG_FRAGMENT_TEMPLATE.format(preamble=preamble, body=body)
    shader = core.Shader.make(core.Shader.SL_Cg, vertex_code, code)
    if not shader:
        pytest.fail("error compiling shader:\n" + code)

    result = core.Texture("")
    fbprops = core.FrameBufferProperties()
    fbprops.force_hardware = True
    fbprops.set_rgba_bits(8, 8, 8, 8)
    fbprops.srgb_color = False

    engine = gsg.get_engine()
    buffer = engine.make_output(
        gsg.pipe,
        'buffer',
        0,
        fbprops,
        core.WindowProperties.size(core.Texture.up_to_power_2(num_lines + 1), 1),
        core.GraphicsPipe.BF_refuse_window,
        gsg
    )
    buffer.add_render_texture(result, core.GraphicsOutput.RTM_copy_ram, core.GraphicsOutput.RTP_color)
    buffer.set_clear_color_active(True)
    buffer.set_clear_color((0, 0, 0, 0))
    engine.open_windows()

    # Build up the shader inputs
    attrib = core.ShaderAttrib.make(shader)
    for name, value in inputs.items():
        attrib = attrib.set_shader_input(name, value)
    state = state.set_attrib(attrib)

    scene = core.NodePath("root")
    scene.set_attrib(core.DepthTestAttrib.make(core.RenderAttrib.M_always))

    format = core.GeomVertexFormat.get_v3()
    vdata = core.GeomVertexData("tri", format, core.Geom.UH_static)
    vdata.unclean_set_num_rows(3)

    vertex = core.GeomVertexWriter(vdata, "vertex")
    vertex.set_data3(-1, -1, 0)
    vertex.set_data3(3, -1, 0)
    vertex.set_data3(-1, 3, 0)

    tris = core.GeomTriangles(core.Geom.UH_static)
    tris.add_next_vertices(3)

    geom = core.Geom(vdata)
    geom.add_primitive(tris)

    gnode = core.GeomNode("tri")
    gnode.add_geom(geom, state)
    scene.attach_new_node(gnode)
    scene.set_two_sided(True)

    camera = scene.attach_new_node(core.Camera("camera"))
    camera.node().get_lens(0).set_near_far(-10, 10)
    camera.node().set_cull_bounds(core.OmniBoundingVolume())

    region = buffer.make_display_region()
    region.active = True
    region.camera = camera

    try:
        engine.render_frame()
    except AssertionError as exc:
        assert False, "Error executing shader:\n" + code
    finally:
        engine.remove_window(buffer)

    # Download the texture to check whether the assertion triggered.
    triggered = tuple(result.get_ram_image())
    if triggered[0]:
        pytest.fail("control check failed")

    if not all(triggered[4:]):
        count = 0
        lines = body.split('\n')
        formatted = ''
        for i, line in enumerate(lines):
            offset = (i + 1) * 4
            x = triggered[offset + 2] == 0
            y = triggered[offset + 1] == 0
            z = triggered[offset] == 0
            w = triggered[offset + 3] == 0
            if x or y or z or w:
                count += 1
            else:
                continue
            formatted += '=>  ' + line
            components = ''
            if x:
                components += 'x'
            if y:
                components += 'y'
            if z:
                components += 'z'
            if w:
                components += 'w'
            formatted += f'      <= {components} components don\'t match'
            formatted += '\n'
        pytest.fail("{0} Cg assertions triggered:\n{1}".format(count, formatted))


def run_cg_compile_check(gsg, shader_path, expect_fail=False):
    """Compile supplied Cg shader path and check for errors"""
    shader = core.Shader.load(shader_path, core.Shader.SL_Cg)
    # assert shader.is_prepared(gsg.prepared_objects)
    if expect_fail:
        assert shader is None
    else:
        assert shader is not None


def test_cg_compile_error(gsg):
    """Test getting compile errors from bad Cg shaders"""
    shader_path = core.Filename(SHADERS_DIR, 'cg_bad.sha')
    run_cg_compile_check(gsg, shader_path, expect_fail=True)


def test_cg_from_file(gsg):
    """Test compiling Cg shaders from files"""
    shader_path = core.Filename(SHADERS_DIR, 'cg_simple.sha')
    run_cg_compile_check(gsg, shader_path)


def test_cg_test(gsg):
    "Test to make sure that the Cg tests work correctly."

    run_cg_test(gsg, "assert(true);")


def test_cg_test_fail(gsg):
    "Same as above, but making sure that the failure case works correctly."

    with pytest.raises(Failed):
        run_cg_test(gsg, "assert(false);")


def test_cg_sampler(gsg):
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
    run_cg_test(gsg, code, preamble, {'tex1': tex1, 'tex2': tex2, 'tex3': tex3})


def test_cg_int(gsg):
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
    run_cg_test(gsg, code, preamble, inputs)


def test_cg_state_material(gsg):
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

    run_cg_test(gsg, code, preamble, state=node.get_state())


def test_cg_state_fog(gsg):
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

    run_cg_test(gsg, code, preamble, state=node.get_state())


def test_cg_texpad_texpix(gsg):
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

    run_cg_test(gsg, code, preamble, inputs={"test": tex})


def test_cg_alight(gsg):
    alight = core.AmbientLight("alight")
    alight.set_color((1, 2, 3, 4))
    np = core.NodePath(alight)

    preamble = """
    uniform float4 alight_test;
    """
    code = """
    assert(alight_test == float4(1, 2, 3, 4));
    """

    run_cg_test(gsg, code, preamble, inputs={"test": np})


def test_cg_satten(gsg):
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

    run_cg_test(gsg, code, preamble, inputs={"test": np})
