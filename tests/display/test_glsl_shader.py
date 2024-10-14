from panda3d import core
import os
import struct
import pytest
from _pytest.outcomes import Failed


SHADERS_DIR = core.Filename.from_os_specific(os.path.dirname(__file__))


# This is the template for the compute shader that is used by run_glsl_test.
# It defines an assert() macro that writes failures to a buffer, indexed by
# line number.
# The reset() function serves to prevent the _triggered variable from being
# optimized out in the case that the assertions are being optimized out.
GLSL_COMPUTE_TEMPLATE = """#version {version}
{extensions}

layout(local_size_x = 1, local_size_y = 1) in;

{preamble}

layout(r8ui) uniform writeonly uimageBuffer _triggered;

void _reset() {{
    imageStore(_triggered, 0, uvec4(1));
    memoryBarrier();
}}

void _assert(bool cond, int line) {{
    if (!cond) {{
        imageStore(_triggered, line, uvec4(1));
    }}
}}

#define assert(cond) _assert(cond, __LINE__ - line_offset)

void main() {{
    _reset();
    const int line_offset = __LINE__;
{body}
}}
"""

# This is a version that uses a vertex and fragment shader instead.  This is
# slower to set up, but it works even when compute shaders are not supported.
# The shader is rendered on a fullscreen triangle to a texture, where each
# pixel represents one line of the code.  The assert writes the result to the
# output color if the current fragment matches the line number of that assert.
# The first pixel is used as a control, to check that the shader has run.
GLSL_VERTEX_TEMPLATE = """#version {version}

in vec4 p3d_Vertex;

void main() {{
    gl_Position = p3d_Vertex;
}}
"""

GLSL_FRAGMENT_TEMPLATE = """#version {version}
{extensions}

{preamble}

layout(location = 0) out vec4 p3d_FragColor;

void _reset() {{
    p3d_FragColor = vec4(0, 0, 0, 0);

    if (int(gl_FragCoord.x) == 0) {{
        p3d_FragColor = vec4(1, 1, 1, 1);
    }}
}}

void _assert(bool cond, int line) {{
    if (int(gl_FragCoord.x) == line) {{
        p3d_FragColor = vec4(!cond, !cond, !cond, !cond);
    }}
}}

#define assert(cond) _assert(cond, __LINE__ - line_offset)

void main() {{
    _reset();
    const int line_offset = __LINE__;
{body}
}}
"""


def run_glsl_test(gsg, body, preamble="", inputs={}, version=420, exts=set(),
                  state=core.RenderState.make_empty()):
    """ Runs a GLSL test on the given GSG.  The given body is executed in the
    main function and should call assert().  The preamble should contain all
    of the shader inputs. """

    if not gsg.supports_basic_shaders:
        pytest.skip("shaders not supported")

    use_compute = gsg.supports_compute_shaders and \
                  gsg.supports_buffer_texture and \
                  (gsg.supported_shader_capabilities & core.Shader.C_image_load_store) != 0

    missing_exts = sorted(ext for ext in exts if not gsg.has_extension(ext))
    if missing_exts:
        pytest.skip("missing extensions: " + ' '.join(missing_exts))

    if use_compute:
        exts = exts | {'GL_ARB_compute_shader', 'GL_ARB_shader_image_load_store'}

    extensions = ''
    for ext in exts:
        extensions += '#extension {ext} : require\n'.format(ext=ext)

    __tracebackhide__ = True

    preamble = preamble.strip()
    body = body.rstrip().lstrip('\n')

    if use_compute:
        code = GLSL_COMPUTE_TEMPLATE.format(version=version, extensions=extensions, preamble=preamble, body=body)
        shader = core.Shader.make_compute(core.Shader.SL_GLSL, code)
    else:
        vertex_code = GLSL_VERTEX_TEMPLATE.format(version=version, extensions=extensions, preamble=preamble, body=body)
        code = GLSL_FRAGMENT_TEMPLATE.format(version=version, extensions=extensions, preamble=preamble, body=body)
        shader = core.Shader.make(core.Shader.SL_GLSL, vertex_code, code)

    if not shader:
        pytest.fail("error compiling shader:\n" + code)

    unsupported_caps = shader.get_used_capabilities() & ~gsg.supported_shader_capabilities
    if unsupported_caps != 0:
        stream = core.StringStream()
        core.ShaderEnums.output_capabilities(stream, unsupported_caps)
        pytest.skip("unsupported capabilities: " + stream.data.decode('ascii'))

    num_lines = body.count('\n') + 1

    # Create a buffer to hold the results of the assertion.  We use one texel
    # per line of shader code, so we can show which lines triggered.
    engine = gsg.get_engine()
    result = core.Texture("")
    if use_compute:
        result.set_clear_color((0, 0, 0, 0))
        result.setup_buffer_texture(num_lines + 1, core.Texture.T_unsigned_byte,
                                    core.Texture.F_r8i, core.GeomEnums.UH_static)
    else:
        fbprops = core.FrameBufferProperties()
        fbprops.force_hardware = True
        fbprops.set_rgba_bits(8, 8, 8, 8)
        fbprops.srgb_color = False

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
    if use_compute:
        attrib = attrib.set_shader_input('_triggered', result)
    state = state.set_attrib(attrib)

    # Run the shader.
    if use_compute:
        try:
            engine.dispatch_compute((1, 1, 1), state, gsg)
        except AssertionError as exc:
            assert False, "Error executing compute shader:\n" + code
    else:
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
    if use_compute:
        success = engine.extract_texture_data(result, gsg)
        assert success

    triggered = result.get_ram_image()
    if use_compute:
        triggered = tuple(triggered)
    else:
        triggered = tuple(memoryview(triggered).cast('I'))

    if not triggered[0]:
        pytest.fail("control check failed")

    if any(triggered[1:]):
        count = len(triggered) - triggered.count(0) - 1
        lines = body.split('\n')
        formatted = ''
        for i, line in enumerate(lines):
            if triggered[i + 1]:
                formatted += '=>  ' + line + '\n'
            else:
                formatted += '    ' + line + '\n'
        pytest.fail("{0} GLSL assertions triggered:\n{1}".format(count, formatted))


def run_glsl_compile_check(gsg, vert_path, frag_path, expect_fail=False):
    """Compile supplied GLSL shader paths and check for errors"""
    shader = core.Shader.load(core.Shader.SL_GLSL, vert_path, frag_path)
    if expect_fail:
        assert shader is None
        return

    assert shader is not None

    if not gsg.supports_glsl:
        expect_fail = True

    shader.prepare_now(gsg.prepared_objects, gsg)
    assert shader.is_prepared(gsg.prepared_objects)
    if expect_fail:
        assert shader.get_error_flag()
    else:
        assert not shader.get_error_flag()


def test_glsl_test(gsg):
    "Test to make sure that the GLSL tests work correctly."

    run_glsl_test(gsg, "assert(true);")


def test_glsl_test_fail(gsg):
    "Same as above, but making sure that the failure case works correctly."

    with pytest.raises(Failed):
        run_glsl_test(gsg, "assert(false);")


def test_glsl_sampler(gsg):
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
    assert(texture(tex1, 0) == vec4(0, 2 / 255.0, 1, 1));
    assert(texture(tex2, vec2(0, 0)) == vec4(1.0, 2.0, -3.14, 0.0));
    assert(texture(tex3, vec3(0, 0, 0)).r == 0.5);
    """
    run_glsl_test(gsg, code, preamble, {'tex1': tex1, 'tex2': tex2, 'tex3': tex3})


def test_glsl_isampler(gsg):
    from struct import pack

    tex1 = core.Texture("")
    tex1.setup_1d_texture(1, core.Texture.T_byte, core.Texture.F_rgba8i)
    tex1.set_ram_image(pack('bbbb', 0, 1, 2, 3))

    tex2 = core.Texture("")
    tex2.setup_2d_texture(1, 1, core.Texture.T_short, core.Texture.F_r16i)
    tex2.set_ram_image(pack('h', 4))

    tex3 = core.Texture("")
    tex3.setup_3d_texture(1, 1, 1, core.Texture.T_int, core.Texture.F_r32i)
    tex3.set_ram_image(pack('i', 5))

    preamble = """
    uniform isampler1D tex1;
    uniform isampler2D tex2;
    uniform isampler3D tex3;
    """
    code = """
    assert(texelFetch(tex1, 0, 0) == ivec4(0, 1, 2, 3));
    assert(texelFetch(tex2, ivec2(0, 0), 0) == ivec4(4, 0, 0, 1));
    assert(texelFetch(tex3, ivec3(0, 0, 0), 0) == ivec4(5, 0, 0, 1));
    """
    run_glsl_test(gsg, code, preamble, {'tex1': tex1, 'tex2': tex2, 'tex3': tex3})


def test_glsl_usampler(gsg):
    from struct import pack

    tex1 = core.Texture("")
    tex1.setup_1d_texture(1, core.Texture.T_unsigned_byte, core.Texture.F_rgba8i)
    tex1.set_ram_image(pack('BBBB', 0, 1, 2, 3))

    tex2 = core.Texture("")
    tex2.setup_2d_texture(1, 1, core.Texture.T_unsigned_short, core.Texture.F_r16i)
    tex2.set_ram_image(pack('H', 4))

    tex3 = core.Texture("")
    tex3.setup_3d_texture(1, 1, 1, core.Texture.T_unsigned_int, core.Texture.F_r32i)
    tex3.set_ram_image(pack('I', 5))

    preamble = """
    uniform usampler1D tex1;
    uniform usampler2D tex2;
    uniform usampler3D tex3;
    """
    code = """
    assert(texelFetch(tex1, 0, 0) == uvec4(0, 1, 2, 3));
    assert(texelFetch(tex2, ivec2(0, 0), 0) == uvec4(4, 0, 0, 1));
    assert(texelFetch(tex3, ivec3(0, 0, 0), 0) == uvec4(5, 0, 0, 1));
    """
    run_glsl_test(gsg, code, preamble, {'tex1': tex1, 'tex2': tex2, 'tex3': tex3})


def test_glsl_image(gsg):
    tex1 = core.Texture("")
    tex1.setup_1d_texture(1, core.Texture.T_unsigned_byte, core.Texture.F_rgba8)
    tex1.set_clear_color((0, 2 / 255.0, 1, 1))

    tex2 = core.Texture("")
    tex2.setup_2d_texture(1, 1, core.Texture.T_float, core.Texture.F_rgba32)
    tex2.set_clear_color((1.0, 2.0, -3.14, 0.0))

    preamble = """
    layout(rgba8) uniform image1D tex1;
    layout(rgba32f) uniform image2D tex2;
    """
    code = """
    assert(imageLoad(tex1, 0) == vec4(0, 2 / 255.0, 1, 1));
    assert(imageLoad(tex2, ivec2(0, 0)) == vec4(1.0, 2.0, -3.14, 0.0));
    """
    run_glsl_test(gsg, code, preamble, {'tex1': tex1, 'tex2': tex2})


def test_glsl_iimage(gsg):
    from struct import pack

    tex1 = core.Texture("")
    tex1.setup_1d_texture(1, core.Texture.T_byte, core.Texture.F_rgba8i)
    tex1.set_ram_image(pack('bbbb', 0, 1, 2, 3))

    tex2 = core.Texture("")
    tex2.setup_2d_texture(1, 1, core.Texture.T_short, core.Texture.F_r16i)
    tex2.set_ram_image(pack('h', 4))

    tex3 = core.Texture("")
    tex3.setup_3d_texture(1, 1, 1, core.Texture.T_int, core.Texture.F_r32i)
    tex3.set_ram_image(pack('i', 5))

    preamble = """
    layout(rgba8i) uniform iimage1D tex1;
    layout(r16i) uniform iimage2D tex2;
    layout(r32i) uniform iimage3D tex3;
    """
    code = """
    assert(imageLoad(tex1, 0) == ivec4(0, 1, 2, 3));
    assert(imageLoad(tex2, ivec2(0, 0)) == ivec4(4, 0, 0, 1));
    assert(imageLoad(tex3, ivec3(0, 0, 0)) == ivec4(5, 0, 0, 1));
    """
    run_glsl_test(gsg, code, preamble, {'tex1': tex1, 'tex2': tex2, 'tex3': tex3})


def test_glsl_uimage(gsg):
    from struct import pack

    tex1 = core.Texture("")
    tex1.setup_1d_texture(1, core.Texture.T_unsigned_byte, core.Texture.F_rgba8i)
    tex1.set_ram_image(pack('BBBB', 0, 1, 2, 3))

    tex2 = core.Texture("")
    tex2.setup_2d_texture(1, 1, core.Texture.T_unsigned_short, core.Texture.F_r16i)
    tex2.set_ram_image(pack('H', 4))

    tex3 = core.Texture("")
    tex3.setup_3d_texture(1, 1, 1, core.Texture.T_unsigned_int, core.Texture.F_r32i)
    tex3.set_ram_image(pack('I', 5))

    preamble = """
    layout(rgba8ui) uniform uimage1D tex1;
    layout(r16ui) uniform uimage2D tex2;
    layout(r32ui) uniform uimage3D tex3;
    """
    code = """
    assert(imageLoad(tex1, 0) == uvec4(0, 1, 2, 3));
    assert(imageLoad(tex2, ivec2(0, 0)) == uvec4(4, 0, 0, 1));
    assert(imageLoad(tex3, ivec3(0, 0, 0)) == uvec4(5, 0, 0, 1));
    """
    run_glsl_test(gsg, code, preamble, {'tex1': tex1, 'tex2': tex2, 'tex3': tex3})


def test_glsl_ssbo(gsg):
    return
    from struct import pack
    num1 = pack('<i', 1234567)
    num2 = pack('<i', -1234567)
    buffer1 = core.ShaderBuffer("buffer1", num1, core.GeomEnums.UH_static)
    buffer2 = core.ShaderBuffer("buffer2", num2, core.GeomEnums.UH_static)
    buffer3 = core.ShaderBuffer("buffer3", 4, core.GeomEnums.UH_static)

    preamble = """
    layout(std430, binding=0) readonly buffer buffer1 {
        int value1;
    };
    layout(std430, binding=1) buffer buffer2 {
        readonly int value2;
    };
    layout(std430, binding=3) buffer buffer3 {
        writeonly int value3;
        int value4;
    };
    """
    # Assigning value3 to 999 first proves buffers aren't accidentally aliased
    code = """
    value3 = 999;
    assert(value1 == 1234567);
    assert(value2 == -1234567);
    """
    run_glsl_test(gsg, code, preamble,
                  {'buffer1': buffer1, 'buffer2': buffer2, 'buffer3': buffer3},
                  version=430)


def test_glsl_ssbo_runtime_length(gsg):
    return
    from struct import pack
    nums = pack('<ii', 1234, 5678)
    ssbo = core.ShaderBuffer("ssbo", nums, core.GeomEnums.UH_static)

    preamble = """
    layout(std430, binding=0) buffer ssbo {
        int values[];
    };
    """
    code = """
    assert(values.length() == 2);
    assert(values[0] == 1234);
    assert(values[1] == 5678);
    """
    run_glsl_test(gsg, code, preamble, {'ssbo': ssbo}, version=430)


def test_glsl_float(gsg):
    inputs = dict(
        zero=0,
        a=1.23,
        b=-829.123,
    )
    preamble = """
    uniform float zero;
    uniform float a;
    uniform float b;
    """
    code = """
    assert(zero == 0);
    assert(abs(a - 1.23) < 0.001);
    assert(abs(b - -829.123) < 0.001);
    """
    run_glsl_test(gsg, code, preamble, inputs)


def test_glsl_int(gsg):
    inputs = dict(
        zero=0,
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
    run_glsl_test(gsg, code, preamble, inputs)


def test_glsl_uint(gsg):
    #TODO: fix passing uints greater than intmax
    inputs = dict(
        zero=0,
        intmax=0x7fffffff,
    )
    preamble = """
    uniform uint zero;
    uniform uint intmax;
    """
    code = """
    assert(zero == 0u);
    assert(intmax == 0x7fffffffu);
    """
    run_glsl_test(gsg, code, preamble, inputs)


#@pytest.mark.xfail(reason="https://github.com/KhronosGroup/SPIRV-Tools/issues/3387")
def test_glsl_bool(gsg):
    flags = dict(
        flag1=False,
        flag2=0,
        flag3=0.0,
        flag4=True,
        flag5=1,
        flag6=3,
    )
    preamble = """
    uniform bool flag1;
    uniform bool flag2;
    uniform bool flag3;
    uniform bool flag4;
    uniform bool flag5;
    uniform bool flag6;
    """
    code = """
    assert(!flag1);
    assert(!flag2);
    assert(!flag3);
    assert(flag4);
    assert(flag5);
    assert(flag6);
    """
    run_glsl_test(gsg, code, preamble, flags)


def test_glsl_mat3(gsg):
    param1 = core.LMatrix4f(core.LMatrix3f(1, 2, 3, 4, 5, 6, 7, 8, 9))
    param2 = core.LMatrix4d(core.LMatrix3d(10, 11, 12, 13, 14, 15, 16, 17, 18))

    param3 = core.NodePath("param3")
    param3.set_mat(core.LMatrix3(19, 20, 21, 22, 23, 24, 25, 26, 27))

    preamble = """
    uniform mat3 param1;
    uniform mat3 param2;
    uniform mat3 param3;
    """
    code = """
    assert(param1[0] == vec3(1, 2, 3));
    assert(param1[1] == vec3(4, 5, 6));
    assert(param1[2] == vec3(7, 8, 9));
    assert(param2[0] == vec3(10, 11, 12));
    assert(param2[1] == vec3(13, 14, 15));
    assert(param2[2] == vec3(16, 17, 18));
    assert(param3[0] == vec3(19, 20, 21));
    assert(param3[1] == vec3(22, 23, 24));
    assert(param3[2] == vec3(25, 26, 27));
    """
    run_glsl_test(gsg, code, preamble,
        {'param1': param1, 'param2': param2, 'param3': param3})


def test_glsl_mat4(gsg):
    param1 = core.LMatrix4f(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16)
    param2 = core.LMatrix4d(17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32)

    param3 = core.NodePath("param3")
    param3.set_mat(core.LMatrix4(
        33, 34, 35, 36,
        37, 38, 39, 40,
        41, 42, 43, 44,
        45, 46, 47, 48))

    preamble = """
    uniform mat4 param1;
    uniform mat4 param2;
    uniform mat4 param3;
    """
    code = """
    assert(param1[0] == vec4(1, 2, 3, 4));
    assert(param1[1] == vec4(5, 6, 7, 8));
    assert(param1[2] == vec4(9, 10, 11, 12));
    assert(param1[3] == vec4(13, 14, 15, 16));
    assert(param2[0] == vec4(17, 18, 19, 20));
    assert(param2[1] == vec4(21, 22, 23, 24));
    assert(param2[2] == vec4(25, 26, 27, 28));
    assert(param2[3] == vec4(29, 30, 31, 32));
    assert(param3[0] == vec4(33, 34, 35, 36));
    assert(param3[1] == vec4(37, 38, 39, 40));
    assert(param3[2] == vec4(41, 42, 43, 44));
    assert(param3[3] == vec4(45, 46, 47, 48));
    """
    run_glsl_test(gsg, code, preamble,
        {'param1': param1, 'param2': param2, 'param3': param3})


def test_glsl_mat3x4(gsg):
    param1 = core.LMatrix4f(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16)
    param2 = core.LMatrix4d(17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32)

    preamble = """
    uniform mat3x4 param1;
    uniform mat3x4 param2;
    """
    code = """
    assert(param1[0] == vec4(1, 2, 3, 4));
    assert(param1[1] == vec4(5, 6, 7, 8));
    assert(param1[2] == vec4(9, 10, 11, 12));
    assert(param2[0] == vec4(17, 18, 19, 20));
    assert(param2[1] == vec4(21, 22, 23, 24));
    assert(param2[2] == vec4(25, 26, 27, 28));
    """
    run_glsl_test(gsg, code, preamble,
        {'param1': param1, 'param2': param2})


def test_glsl_mat4x3(gsg):
    param1 = core.LMatrix4f(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16)
    param2 = core.LMatrix4d(17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32)

    preamble = """
    uniform mat4x3 param1;
    uniform mat4x3 param2;
    uniform mat4x3 param3;
    """
    code = """
    assert(param1[0] == vec3(1, 2, 3));
    assert(param1[1] == vec3(5, 6, 7));
    assert(param1[2] == vec3(9, 10, 11));
    assert(param1[3] == vec3(13, 14, 15));
    assert(param2[0] == vec3(17, 18, 19));
    assert(param2[1] == vec3(21, 22, 23));
    assert(param2[2] == vec3(25, 26, 27));
    assert(param2[3] == vec3(29, 30, 31));
    """
    run_glsl_test(gsg, code, preamble,
        {'param1': param1, 'param2': param2})


def test_glsl_pta_int(gsg):
    pta = core.PTA_int((0, 1, 2, 3))

    preamble = """
    uniform int pta[4];
    """
    code = """
    assert(pta[0] == 0);
    assert(pta[1] == 1);
    assert(pta[2] == 2);
    assert(pta[3] == 3);
    """
    run_glsl_test(gsg, code, preamble, {'pta': pta})


def test_glsl_pta_ivec4(gsg):
    pta = core.PTA_LVecBase4i(((0, 1, 2, 3), (4, 5, 6, 7)))

    preamble = """
    uniform ivec4 pta[2];
    """
    code = """
    assert(pta[0] == ivec4(0, 1, 2, 3));
    assert(pta[1] == ivec4(4, 5, 6, 7));
    """
    run_glsl_test(gsg, code, preamble, {'pta': pta})


@pytest.mark.parametrize("type", (core.PTA_LVecBase3f, core.PTA_LVecBase3d, core.PTA_LVecBase3i))
def test_glsl_pta_vec3(gsg, type):
    pta = type((
        (0, 1, 2),
        (3, 4, 5),
        (6, 7, 8),
    ))

    preamble = """
    uniform vec3 pta[3];
    """
    code = """
    assert(pta[0] == vec3(0, 1, 2));
    assert(pta[1] == vec3(3, 4, 5));
    assert(pta[2] == vec3(6, 7, 8));
    """
    run_glsl_test(gsg, code, preamble, {'pta': pta})


@pytest.mark.parametrize("type", (core.PTA_LVecBase3f, core.PTA_LVecBase3d, core.PTA_LVecBase3i))
def test_glsl_pta_dvec3(gsg, type):
    pta = type((
        (0, 1, 2),
        (3, 4, 5),
        (6, 7, 8),
    ))

    preamble = """
    uniform dvec3 pta[3];
    """
    code = """
    assert(pta[0] == vec3(0, 1, 2));
    assert(pta[1] == vec3(3, 4, 5));
    assert(pta[2] == vec3(6, 7, 8));
    """
    run_glsl_test(gsg, code, preamble, {'pta': pta})


@pytest.mark.parametrize("type", (core.PTA_LVecBase4f, core.PTA_LVecBase4d, core.PTA_LVecBase4i))
def test_glsl_pta_vec4(gsg, type):
    pta = type((
        (0, 1, 2, 3),
        (4, 5, 6, 7),
        (8, 9, 10, 11),
    ))

    preamble = """
    uniform vec4 pta[4];
    """
    code = """
    assert(pta[0] == vec4(0, 1, 2, 3));
    assert(pta[1] == vec4(4, 5, 6, 7));
    assert(pta[2] == vec4(8, 9, 10, 11));
    """
    run_glsl_test(gsg, code, preamble, {'pta': pta})


@pytest.mark.parametrize("type", (core.PTA_LVecBase4f, core.PTA_LVecBase4d, core.PTA_LVecBase4i))
def test_glsl_pta_dvec4(gsg, type):
    pta = type((
        (0, 1, 2, 3),
        (4, 5, 6, 7),
        (8, 9, 10, 11),
    ))

    preamble = """
    uniform dvec4 pta[4];
    """
    code = """
    assert(pta[0] == dvec4(0, 1, 2, 3));
    assert(pta[1] == dvec4(4, 5, 6, 7));
    assert(pta[2] == dvec4(8, 9, 10, 11));
    """
    run_glsl_test(gsg, code, preamble, {'pta': pta})


@pytest.mark.parametrize("type", (core.PTA_LMatrix3f, core.PTA_LMatrix3d))
def test_glsl_pta_mat3(gsg, type):
    pta = type((
        (0, 1, 2, 3, 4, 5, 6, 7, 8),
        (9, 10, 11, 12, 13, 14, 15, 16, 17),
    ))

    preamble = """
    uniform mat3 pta[2];
    """
    code = """
    assert(pta[0][0] == vec3(0, 1, 2));
    assert(pta[0][1] == vec3(3, 4, 5));
    assert(pta[0][2] == vec3(6, 7, 8));
    assert(pta[1][0] == vec3(9, 10, 11));
    assert(pta[1][1] == vec3(12, 13, 14));
    assert(pta[1][2] == vec3(15, 16, 17));
    """
    run_glsl_test(gsg, code, preamble, {'pta': pta})


@pytest.mark.parametrize("type", (core.PTA_LMatrix4f, core.PTA_LMatrix4d))
def test_glsl_pta_mat4(gsg, type):
    pta = type((
        (0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15),
        (16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31),
    ))

    preamble = """
    uniform mat4 pta[2];
    """
    code = """
    assert(pta[0][0] == vec4(0, 1, 2, 3));
    assert(pta[0][1] == vec4(4, 5, 6, 7));
    assert(pta[0][2] == vec4(8, 9, 10, 11));
    assert(pta[0][3] == vec4(12, 13, 14, 15));
    assert(pta[1][0] == vec4(16, 17, 18, 19));
    assert(pta[1][1] == vec4(20, 21, 22, 23));
    assert(pta[1][2] == vec4(24, 25, 26, 27));
    assert(pta[1][3] == vec4(28, 29, 30, 31));
    """
    run_glsl_test(gsg, code, preamble, {'pta': pta})


def test_glsl_param_vec4(gsg):
    param = core.ParamVecBase4((0, 1, 2, 3))

    preamble = """
    uniform vec4 param;
    """
    code = """
    assert(param.x == 0.0);
    assert(param.y == 1.0);
    assert(param.z == 2.0);
    assert(param.w == 3.0);
    """
    run_glsl_test(gsg, code, preamble, {'param': param})


def test_glsl_param_ivec4(gsg):
    param = core.ParamVecBase4i((0, 1, 2, 3))

    preamble = """
    uniform ivec4 param;
    """
    code = """
    assert(param.x == 0);
    assert(param.y == 1);
    assert(param.z == 2);
    assert(param.w == 3);
    """
    run_glsl_test(gsg, code, preamble, {'param': param})


def test_glsl_struct(gsg):
    preamble = """
    uniform struct TestStruct {
        vec3 a;
        float b;
        sampler2D c;
        float unused;
        vec3 d[2];
        vec2 e;
        sampler2D f;
    } test;
    """
    code = """
    assert(test.a == vec3(1, 2, 3));
    assert(test.b == 4);
    assert(texture(test.c, vec2(0, 0)).r == 5);
    assert(test.d[0] == vec3(6, 7, 8));
    assert(test.d[1] == vec3(9, 10, 11));
    assert(test.e == vec2(12, 13));
    assert(texture(test.f, vec2(0, 0)).r == 14);
    """
    tex_c = core.Texture('c')
    tex_c.setup_2d_texture(1, 1, core.Texture.T_float, core.Texture.F_r32)
    tex_c.set_clear_color((5, 0, 0, 0))
    tex_f = core.Texture('f')
    tex_f.setup_2d_texture(1, 1, core.Texture.T_float, core.Texture.F_r32)
    tex_f.set_clear_color((14, 0, 0, 0))
    run_glsl_test(gsg, code, preamble, {
        'test.unused': 0,
        'test.a': (1, 2, 3),
        'test.b': 4,
        'test.c': tex_c,
        'test.d': [(6, 7, 8), (9, 10, 11)],
        'test.e': [12, 13],
        'test.f': tex_f,
    })


def test_glsl_struct_nested(gsg):
    preamble = """
    struct TestSubStruct1 {
        float a;
        float b;
    };
    struct TestSubStruct2 {
        float unused;
        sampler2D a;
        vec2 b;
    };
    uniform struct TestStruct {
        vec3 a;
        TestSubStruct1 b;
        TestSubStruct2 c;
        float d;
    } test;
    """
    code = """
    assert(test.a == vec3(1, 2, 3));
    assert(test.b.a == 4);
    assert(test.b.b == 5);
    assert(texture(test.c.a, vec2(0, 0)).r == 6);
    assert(test.c.b == vec2(7, 8));
    assert(test.d == 9);
    """
    tex_c_a = core.Texture()
    tex_c_a.setup_2d_texture(1, 1, core.Texture.T_float, core.Texture.F_r32)
    tex_c_a.set_clear_color((6, 0, 0, 0))
    run_glsl_test(gsg, code, preamble, {
        'test.unused': 0,
        'test.a': (1, 2, 3),
        'test.b.a': 4,
        'test.b.b': 5,
        'test.c.unused': 0,
        'test.c.a': tex_c_a,
        'test.c.b': (7, 8),
        'test.d': 9,
    })


def test_glsl_struct_array(gsg):
    preamble = """
    uniform struct TestStruct {
        vec3 a;
        sampler2D b;
        float unused;
        float c;
    } test[2];
    """
    code = """
    assert(test[0].a == vec3(1, 2, 3));
    assert(texture(test[0].b, vec2(0, 0)).r == 4);
    assert(test[0].c == 5);
    assert(test[1].a == vec3(6, 7, 8));
    assert(texture(test[1].b, vec2(0, 0)).r == 9);
    assert(test[1].c == 10);
    """
    tex_0_b = core.Texture()
    tex_0_b.setup_2d_texture(1, 1, core.Texture.T_float, core.Texture.F_r32)
    tex_0_b.set_clear_color((4, 0, 0, 0))
    tex_1_b = core.Texture()
    tex_1_b.setup_2d_texture(1, 1, core.Texture.T_float, core.Texture.F_r32)
    tex_1_b.set_clear_color((9, 0, 0, 0))
    run_glsl_test(gsg, code, preamble, {
        'test[0].unused': 0,
        'test[0].a': (1, 2, 3),
        'test[0].b': tex_0_b,
        'test[0].c': 5,
        'test[1].unused': 0,
        'test[1].a': (6, 7, 8),
        'test[1].b': tex_1_b,
        'test[1].c': 10,
    })


def test_glsl_struct_pseudo_light(gsg):
    # Something that looks like a named light source, but isn't one at all
    preamble = """
    struct FakeLightParameters {
      vec4 specular;
      vec4 position;
      vec3 attenuation;
      float constantAttenuation;
      float radius;
    };
    uniform FakeLightParameters test;
    """
    code = """
    assert(test.specular == vec4(1, 2, 3, 4));
    assert(test.position == vec4(5, 6, 7, 8));
    assert(test.attenuation == vec3(9, 10, 11));
    assert(test.constantAttenuation == 12);
    assert(test.radius == 13);
    """
    run_glsl_test(gsg, code, preamble, {
        'test.specular': (1, 2, 3, 4),
        'test.position': (5, 6, 7, 8),
        'test.attenuation': (9, 10, 11),
        'test.constantAttenuation': 12,
        'test.radius': 13,
    })


def test_glsl_light(gsg):
    preamble = """
    uniform struct p3d_LightSourceParameters {
        vec4 color;
        vec3 ambient;
        vec4 diffuse;
        vec4 specular;
        vec3 position;
        vec4 halfVector;
        vec4 spotDirection;
        float spotCutoff;
        float spotCosCutoff;
        float spotExponent;
        vec3 attenuation;
        float constantAttenuation;
        float linearAttenuation;
        float quadraticAttenuation;
    } plight;
    """
    code = """
    assert(plight.color == vec4(1, 2, 3, 4));
    assert(plight.ambient == vec3(0, 0, 0));
    assert(plight.diffuse == vec4(1, 2, 3, 4));
    assert(plight.specular == vec4(5, 6, 7, 8));
    assert(plight.position == vec3(9, 10, 11));
    assert(plight.spotCutoff == 180);
    assert(plight.spotCosCutoff == -1);
    assert(plight.spotExponent == 0);
    assert(plight.attenuation == vec3(12, 13, 14));
    assert(plight.constantAttenuation == 12);
    assert(plight.linearAttenuation == 13);
    assert(plight.quadraticAttenuation == 14);
    """
    plight = core.PointLight("plight")
    plight.color = (1, 2, 3, 4)
    plight.specular_color = (5, 6, 7, 8)
    plight.transform = core.TransformState.make_pos((9, 10, 11))
    plight.attenuation = (12, 13, 14)

    run_glsl_test(gsg, code, preamble, {
        'plight': core.NodePath(plight),
    })


def test_glsl_named_light_source(gsg):
    spot = core.Spotlight("spot")
    spot.get_lens().set_fov(90, 90)
    spot.set_color((1, 2, 3, 4))
    spot.set_specular_color((5, 6, 7, 8))

    preamble = """
    struct p3d_LightSourceParameters {
      vec4 color;
      vec4 specular;
    };
    uniform p3d_LightSourceParameters spot;
    """
    code = """
    assert(spot.color == vec4(1, 2, 3, 4));
    assert(spot.specular == vec4(5, 6, 7, 8));
    """
    run_glsl_test(gsg, code, preamble, {'spot': core.NodePath(spot)})


def test_glsl_state_light(gsg):
    preamble = """
    uniform struct p3d_LightSourceParameters {
        vec4 color;
        vec3 ambient;
        vec4 diffuse;
        vec4 specular;
        vec4 position;
        vec4 halfVector;
        vec4 spotDirection;
        float spotCutoff;
        float spotCosCutoff;
        float spotExponent;
        vec3 attenuation;
        float constantAttenuation;
        float linearAttenuation;
        float quadraticAttenuation;
    } p3d_LightSource[2];
    """
    code = """
    assert(p3d_LightSource[0].color == vec4(1, 2, 3, 4));
    assert(p3d_LightSource[0].ambient == vec3(0, 0, 0));
    assert(p3d_LightSource[0].diffuse == vec4(1, 2, 3, 4));
    assert(p3d_LightSource[0].specular == vec4(5, 6, 7, 8));
    assert(p3d_LightSource[0].position == vec4(9, 10, 11, 1));
    assert(p3d_LightSource[0].spotCutoff == 180);
    assert(p3d_LightSource[0].spotCosCutoff == -1);
    assert(p3d_LightSource[0].spotExponent == 0);
    assert(p3d_LightSource[0].attenuation == vec3(12, 13, 14));
    assert(p3d_LightSource[0].constantAttenuation == 12);
    assert(p3d_LightSource[0].linearAttenuation == 13);
    assert(p3d_LightSource[0].quadraticAttenuation == 14);
    assert(p3d_LightSource[1].color == vec4(15, 16, 17, 18));
    assert(p3d_LightSource[1].ambient == vec3(0, 0, 0));
    assert(p3d_LightSource[1].diffuse == vec4(15, 16, 17, 18));
    assert(p3d_LightSource[1].specular == vec4(19, 20, 21, 22));
    assert(p3d_LightSource[1].position == vec4(0, 1, 0, 0));
    assert(p3d_LightSource[1].spotCutoff == 180);
    assert(p3d_LightSource[1].spotCosCutoff == -1);
    assert(p3d_LightSource[1].spotExponent == 0);
    assert(p3d_LightSource[1].attenuation == vec3(1, 0, 0));
    assert(p3d_LightSource[1].constantAttenuation == 1);
    assert(p3d_LightSource[1].linearAttenuation == 0);
    assert(p3d_LightSource[1].quadraticAttenuation == 0);
    """
    plight = core.PointLight("plight")
    plight.priority = 0
    plight.color = (1, 2, 3, 4)
    plight.specular_color = (5, 6, 7, 8)
    plight.transform = core.TransformState.make_pos((9, 10, 11))
    plight.attenuation = (12, 13, 14)
    plight_path = core.NodePath(plight)

    dlight = core.DirectionalLight("dlight")
    dlight.priority = -1
    dlight.direction = (0, -1, 0)
    dlight.color = (15, 16, 17, 18)
    dlight.specular_color = (19, 20, 21, 22)
    dlight.transform = core.TransformState.make_pos((23, 24, 25))
    dlight_path = core.NodePath(dlight)

    lattr = core.LightAttrib.make()
    lattr = lattr.add_on_light(plight_path)
    lattr = lattr.add_on_light(dlight_path)
    state = core.RenderState.make(lattr)

    run_glsl_test(gsg, code, preamble, state=state)


def test_glsl_state_light_source(gsg):
    spot = core.Spotlight("spot")
    spot.priority = 3
    spot.get_lens().set_fov(120, 120)
    spot.set_color((1, 2, 3, 4))
    spot.set_specular_color((5, 6, 7, 8))
    spot.attenuation = (23, 24, 25)
    spot.exponent = 26

    dire = core.DirectionalLight("dire")
    dire.priority = 2
    dire.set_color((9, 10, 11, 12))
    dire.set_specular_color((13, 14, 15, 16))
    dire.direction = (17, 18, 19)

    preamble = """
    struct p3d_LightSourceParameters {
      vec4 color;
      vec4 specular;
      vec4 ambient;
      vec4 diffuse;
      vec4 position;
      vec3 attenuation;
      float constantAttenuation;
      float linearAttenuation;
      float quadraticAttenuation;
      float spotExponent;
      float spotCosCutoff;
      float spotCutoff;
      mat4 shadowViewMatrix;
    };
    uniform p3d_LightSourceParameters p3d_LightSource[3];
    """
    code = """
    assert(p3d_LightSource[0].color == vec4(1, 2, 3, 4));
    assert(p3d_LightSource[0].specular == vec4(5, 6, 7, 8));
    assert(p3d_LightSource[0].ambient == vec4(0, 0, 0, 1));
    assert(p3d_LightSource[0].diffuse == vec4(1, 2, 3, 4));
    assert(p3d_LightSource[0].position == vec4(20, 21, 22, 1));
    assert(p3d_LightSource[0].attenuation == vec3(23, 24, 25));
    assert(p3d_LightSource[0].constantAttenuation == 23);
    assert(p3d_LightSource[0].linearAttenuation == 24);
    assert(p3d_LightSource[0].quadraticAttenuation == 25);
    assert(p3d_LightSource[0].spotExponent == 26);
    assert(p3d_LightSource[0].spotCosCutoff > 0.499);
    assert(p3d_LightSource[0].spotCosCutoff < 0.501);
    assert(p3d_LightSource[0].spotCutoff == 60);
    assert(p3d_LightSource[0].shadowViewMatrix[0][0] > 0.2886);
    assert(p3d_LightSource[0].shadowViewMatrix[0][0] < 0.2887);
    assert(p3d_LightSource[0].shadowViewMatrix[0][1] == 0);
    assert(p3d_LightSource[0].shadowViewMatrix[0][2] == 0);
    assert(p3d_LightSource[0].shadowViewMatrix[0][3] == 0);
    assert(p3d_LightSource[0].shadowViewMatrix[1][0] == 0);
    assert(p3d_LightSource[0].shadowViewMatrix[1][1] > 0.2886);
    assert(p3d_LightSource[0].shadowViewMatrix[1][1] < 0.2887);
    assert(p3d_LightSource[0].shadowViewMatrix[1][2] == 0);
    assert(p3d_LightSource[0].shadowViewMatrix[1][3] == 0);
    //assert(p3d_LightSource[0].shadowViewMatrix[2][0] == -0.5);
    //assert(p3d_LightSource[0].shadowViewMatrix[2][1] == -0.5);
    assert(p3d_LightSource[0].shadowViewMatrix[2][2] > -1.00002);
    //assert(p3d_LightSource[0].shadowViewMatrix[2][2] < -1.0);
    //assert(p3d_LightSource[0].shadowViewMatrix[2][3] == -1);
    assert(p3d_LightSource[0].shadowViewMatrix[3][0] > -16.2736);
    assert(p3d_LightSource[0].shadowViewMatrix[3][0] < -16.2734);
    assert(p3d_LightSource[0].shadowViewMatrix[3][1] > -16.8510);
    assert(p3d_LightSource[0].shadowViewMatrix[3][1] < -16.8508);
    assert(p3d_LightSource[0].shadowViewMatrix[3][2] > -22.0003);
    assert(p3d_LightSource[0].shadowViewMatrix[3][2] < -22.0001);
    assert(p3d_LightSource[0].shadowViewMatrix[3][3] > -21.0001);
    assert(p3d_LightSource[0].shadowViewMatrix[3][3] < -20.9999);
    assert(p3d_LightSource[1].color == vec4(9, 10, 11, 12));
    assert(p3d_LightSource[1].specular == vec4(13, 14, 15, 16));
    assert(p3d_LightSource[1].diffuse == vec4(9, 10, 11, 12));
    assert(p3d_LightSource[1].ambient == vec4(0, 0, 0, 1));
    assert(p3d_LightSource[1].position == vec4(-17, -18, -19, 0));
    assert(p3d_LightSource[1].attenuation == vec3(1, 0, 0));
    assert(p3d_LightSource[1].constantAttenuation == 1);
    assert(p3d_LightSource[1].linearAttenuation == 0);
    assert(p3d_LightSource[1].quadraticAttenuation == 0);
    assert(p3d_LightSource[1].spotExponent == 0);
    assert(p3d_LightSource[1].spotCosCutoff == -1);
    assert(p3d_LightSource[2].color == vec4(0, 0, 0, 1));
    assert(p3d_LightSource[2].specular == vec4(0, 0, 0, 1));
    assert(p3d_LightSource[2].diffuse == vec4(0, 0, 0, 1));
    assert(p3d_LightSource[2].ambient == vec4(0, 0, 0, 1));
    assert(p3d_LightSource[2].position == vec4(0, 0, 1, 0));
    assert(p3d_LightSource[2].attenuation == vec3(1, 0, 0));
    assert(p3d_LightSource[2].constantAttenuation == 1);
    assert(p3d_LightSource[2].linearAttenuation == 0);
    assert(p3d_LightSource[2].quadraticAttenuation == 0);
    assert(p3d_LightSource[2].spotExponent == 0);
    assert(p3d_LightSource[2].spotCosCutoff == -1);
    """

    node = core.NodePath("state")
    spot_path = node.attach_new_node(spot)
    spot_path.set_pos(20, 21, 22)
    node.set_light(spot_path)

    dire_path = node.attach_new_node(dire)
    node.set_light(dire_path)

    run_glsl_test(gsg, code, preamble, state=node.get_state())


def test_glsl_state_material(gsg):
    mat = core.Material("mat")
    mat.ambient = (1, 2, 3, 4)
    mat.diffuse = (5, 6, 7, 8)
    mat.emission = (9, 10, 11, 12)
    mat.specular = (13, 14, 15, 0)
    mat.shininess = 16
    mat.metallic = 0.5
    mat.refractive_index = 21

    preamble = """
    struct p3d_MaterialParameters {
      vec4 ambient;
      vec4 diffuse;
      vec4 emission;
      vec3 specular;
      float shininess;
      float metallic;
      float refractiveIndex;
    };
    uniform p3d_MaterialParameters p3d_Material;
    """
    code = """
    assert(p3d_Material.ambient == vec4(1, 2, 3, 4));
    assert(p3d_Material.diffuse == vec4(5, 6, 7, 8));
    assert(p3d_Material.emission == vec4(9, 10, 11, 12));
    assert(p3d_Material.specular == vec3(13, 14, 15));
    assert(p3d_Material.shininess == 16);
    assert(p3d_Material.metallic == 0.5);
    assert(p3d_Material.refractiveIndex == 21);
    """

    node = core.NodePath("state")
    node.set_material(mat)

    run_glsl_test(gsg, code, preamble, state=node.get_state())


def test_glsl_state_material_pbr(gsg):
    mat = core.Material("mat")
    mat.base_color = (1, 2, 3, 4)
    mat.emission = (9, 10, 11, 12)
    mat.roughness = 16
    mat.metallic = 0.5
    mat.refractive_index = 21

    preamble = """
    struct p3d_MaterialParameters {
      vec4 baseColor;
      vec4 emission;
      float metallic;
      float refractiveIndex;
      float roughness;
    };
    uniform p3d_MaterialParameters p3d_Material;
    """
    code = """
    assert(p3d_Material.baseColor == vec4(1, 2, 3, 4));
    assert(p3d_Material.emission == vec4(9, 10, 11, 12));
    assert(p3d_Material.roughness == 16);
    assert(p3d_Material.metallic == 0.5);
    assert(p3d_Material.refractiveIndex == 21);
    """

    node = core.NodePath("state")
    node.set_material(mat)

    run_glsl_test(gsg, code, preamble, state=node.get_state())


def test_glsl_state_fog(gsg):
    fog = core.Fog("fog")
    fog.color = (1, 2, 3, 4)
    fog.exp_density = 0.5
    fog.set_linear_range(6, 10)

    preamble = """
    struct p3d_FogParameters {
      vec4 color;
      float density;
      float start;
      float end;
      float scale;
    };
    uniform p3d_FogParameters p3d_Fog;
    """
    code = """
    assert(p3d_Fog.color == vec4(1, 2, 3, 4));
    assert(p3d_Fog.density == 0.5);
    assert(p3d_Fog.start == 6);
    assert(p3d_Fog.end == 10);
    assert(p3d_Fog.scale == 0.25);
    """

    node = core.NodePath("state")
    node.set_fog(fog)

    run_glsl_test(gsg, code, preamble, state=node.get_state())


def test_glsl_state_texture(gsg):
    def gen_texture(v):
        tex = core.Texture(f"tex{v}")
        tex.setup_2d_texture(1, 1, core.Texture.T_unsigned_byte, core.Texture.F_red)
        tex.set_clear_color((v / 255.0, 0, 0, 0))
        return tex

    np = core.NodePath("test")

    ts1 = core.TextureStage("ts1")
    ts1.sort = 10
    ts1.mode = core.TextureStage.M_modulate
    np.set_texture(ts1, gen_texture(1))

    ts2 = core.TextureStage("ts2")
    ts2.sort = 20
    ts2.mode = core.TextureStage.M_add
    np.set_texture(ts2, gen_texture(2))

    ts3 = core.TextureStage("ts3")
    ts3.sort = 30
    ts3.mode = core.TextureStage.M_modulate
    np.set_texture(ts3, gen_texture(3))

    ts4 = core.TextureStage("ts4")
    ts4.sort = 40
    ts4.mode = core.TextureStage.M_normal_height
    np.set_texture(ts4, gen_texture(4))

    ts5 = core.TextureStage("ts5")
    ts5.sort = 50
    ts5.mode = core.TextureStage.M_add
    np.set_texture(ts5, gen_texture(5))

    ts6 = core.TextureStage("ts6")
    ts6.sort = 60
    ts6.mode = core.TextureStage.M_normal
    np.set_texture(ts6, gen_texture(6))

    # Do this in multiple passes to stay under sampler limit of 16
    preamble = """
    uniform sampler2D p3d_Texture2;
    uniform sampler2D p3d_Texture0;
    uniform sampler2D p3d_Texture1;
    uniform sampler2D p3d_Texture3;
    uniform sampler2D p3d_Texture4;
    uniform sampler2D p3d_Texture5;
    uniform sampler2D p3d_Texture6;
    uniform sampler2D p3d_Texture[7];
    """
    code = """
    vec2 coord = vec2(0, 0);
    assert(abs(texture(p3d_Texture2, coord).r - 3.0 / 255.0) < 0.001);
    assert(abs(texture(p3d_Texture0, coord).r - 1.0 / 255.0) < 0.001);
    assert(abs(texture(p3d_Texture1, coord).r - 2.0 / 255.0) < 0.001);
    assert(abs(texture(p3d_Texture3, coord).r - 4.0 / 255.0) < 0.001);
    assert(abs(texture(p3d_Texture4, coord).r - 5.0 / 255.0) < 0.001);
    assert(abs(texture(p3d_Texture5, coord).r - 6.0 / 255.0) < 0.001);
    assert(texture(p3d_Texture6, coord).r == 1.0);
    assert(abs(texture(p3d_Texture[0], coord).r - 1.0 / 255.0) < 0.001);
    assert(abs(texture(p3d_Texture[2], coord).r - 3.0 / 255.0) < 0.001);
    assert(abs(texture(p3d_Texture[3], coord).r - 4.0 / 255.0) < 0.001);
    assert(abs(texture(p3d_Texture[1], coord).r - 2.0 / 255.0) < 0.001);
    assert(abs(texture(p3d_Texture[4], coord).r - 5.0 / 255.0) < 0.001);
    assert(abs(texture(p3d_Texture[5], coord).r - 6.0 / 255.0) < 0.001);
    assert(texture(p3d_Texture[6], coord).r == 1.0);
    """

    run_glsl_test(gsg, code, preamble, state=np.get_state())

    preamble = """
    uniform sampler2D p3d_TextureFF[5];
    uniform sampler2D p3d_TextureModulate[3];
    uniform sampler2D p3d_TextureAdd[3];
    uniform sampler2D p3d_TextureNormal[3];
    uniform sampler2D p3d_TextureHeight[2];
    """
    code = """
    vec2 coord = vec2(0, 0);
    assert(abs(texture(p3d_TextureFF[0], coord).r - 1.0 / 255.0) < 0.001);
    assert(abs(texture(p3d_TextureFF[1], coord).r - 2.0 / 255.0) < 0.001);
    assert(abs(texture(p3d_TextureFF[2], coord).r - 3.0 / 255.0) < 0.001);
    assert(abs(texture(p3d_TextureFF[3], coord).r - 5.0 / 255.0) < 0.001);
    assert(texture(p3d_TextureFF[4], coord).r == 1.0);
    assert(abs(texture(p3d_TextureModulate[0], coord).r - 1.0 / 255.0) < 0.001);
    assert(abs(texture(p3d_TextureModulate[1], coord).r - 3.0 / 255.0) < 0.001);
    assert(texture(p3d_TextureModulate[2], coord).r == 1.0);
    assert(abs(texture(p3d_TextureAdd[0], coord).r - 2.0 / 255.0) < 0.001);
    assert(abs(texture(p3d_TextureAdd[1], coord).r - 5.0 / 255.0) < 0.001);
    assert(texture(p3d_TextureAdd[2], coord) == vec4(0.0, 0.0, 0.0, 1.0));
    assert(abs(texture(p3d_TextureNormal[0], coord).r - 4.0 / 255.0) < 0.001);
    assert(abs(texture(p3d_TextureNormal[1], coord).r - 6.0 / 255.0) < 0.001);
    assert(texture(p3d_TextureNormal[2], coord) == vec4(127 / 255.0, 127 / 255.0, 1.0, 0.0));
    assert(texture(p3d_TextureHeight[0], coord).r == 4.0 / 255.0);
    assert(texture(p3d_TextureHeight[1], coord) == vec4(127 / 255.0, 127 / 255.0, 1.0, 0.0));
    """

    run_glsl_test(gsg, code, preamble, state=np.get_state())


def test_glsl_frame_number(gsg):
    clock = core.ClockObject.get_global_clock()
    old_frame_count = clock.get_frame_count()
    try:
        clock.set_frame_count(123)

        preamble = """
        uniform int osg_FrameNumber;
        """
        code = """
        assert(osg_FrameNumber == 123);
        """

        run_glsl_test(gsg, code, preamble)
    finally:
        clock.set_frame_count(old_frame_count)


def test_glsl_write_extract_image_buffer(gsg):
    # Tests that we can write to a buffer texture on the GPU, and then extract
    # the data on the CPU.  We test two textures since there was in the past a
    # where it would only work correctly for one texture.
    tex1 = core.Texture("tex1")
    tex1.set_clear_color(0)
    tex1.setup_buffer_texture(1, core.Texture.T_unsigned_int, core.Texture.F_r32i,
                              core.GeomEnums.UH_static)
    tex2 = core.Texture("tex2")
    tex2.set_clear_color(0)
    tex2.setup_buffer_texture(1, core.Texture.T_int, core.Texture.F_r32i,
                              core.GeomEnums.UH_static)

    preamble = """
    layout(r32ui) uniform uimageBuffer tex1;
    layout(r32i) uniform iimageBuffer tex2;
    """
    code = """
    assert(imageLoad(tex1, 0).r == 0u);
    assert(imageLoad(tex2, 0).r == 0);
    imageStore(tex1, 0, uvec4(123));
    imageStore(tex2, 0, ivec4(-456));
    memoryBarrier();
    assert(imageLoad(tex1, 0).r == 123u);
    assert(imageLoad(tex2, 0).r == -456);
    """

    run_glsl_test(gsg, code, preamble, {'tex1': tex1, 'tex2': tex2})

    engine = core.GraphicsEngine.get_global_ptr()
    assert engine.extract_texture_data(tex1, gsg)
    assert engine.extract_texture_data(tex2, gsg)

    assert struct.unpack('I', tex1.get_ram_image()) == (123,)
    assert struct.unpack('i', tex2.get_ram_image()) == (-456,)


def test_glsl_compile_error(gsg):
    """Test getting compile errors from bad shaders"""
    suffix = ''
    if gsg.pipe.interface_name == "OpenGL" and \
        (gsg.driver_shader_version_major, gsg.driver_shader_version_minor) < (1, 50):
        suffix = '_legacy'
    vert_path = core.Filename(SHADERS_DIR, 'glsl_bad' + suffix + '.vert')
    frag_path = core.Filename(SHADERS_DIR, 'glsl_simple' + suffix + '.frag')
    run_glsl_compile_check(gsg, vert_path, frag_path, expect_fail=True)


def test_glsl_from_file(gsg):
    """Test compiling GLSL shaders from files"""
    suffix = ''
    if gsg.pipe.interface_name == "OpenGL" and \
        (gsg.driver_shader_version_major, gsg.driver_shader_version_minor) < (1, 50):
        suffix = '_legacy'
    vert_path = core.Filename(SHADERS_DIR, 'glsl_simple' + suffix + '.vert')
    frag_path = core.Filename(SHADERS_DIR, 'glsl_simple' + suffix + '.frag')
    run_glsl_compile_check(gsg, vert_path, frag_path)


def test_glsl_includes(gsg):
    """Test preprocessing includes in GLSL shaders"""
    suffix = ''
    if gsg.pipe.interface_name == "OpenGL" and \
        (gsg.driver_shader_version_major, gsg.driver_shader_version_minor) < (1, 50):
        suffix = '_legacy'
    vert_path = core.Filename(SHADERS_DIR, 'glsl_include' + suffix + '.vert')
    frag_path = core.Filename(SHADERS_DIR, 'glsl_simple' + suffix + '.frag')
    run_glsl_compile_check(gsg, vert_path, frag_path)


def test_glsl_includes_angle_nodir(gsg):
    """Test preprocessing includes with angle includes without model-path"""
    suffix = ''
    if gsg.pipe.interface_name == "OpenGL" and \
        (gsg.driver_shader_version_major, gsg.driver_shader_version_minor) < (1, 50):
        suffix = '_legacy'
    vert_path = core.Filename(SHADERS_DIR, 'glsl_include_angle' + suffix + '.vert')
    frag_path = core.Filename(SHADERS_DIR, 'glsl_simple' + suffix + '.frag')
    assert core.Shader.load(core.Shader.SL_GLSL, vert_path, frag_path) is None


@pytest.fixture
def with_current_dir_on_model_path():
    model_path = core.get_model_path()
    model_path.prepend_directory(core.Filename.from_os_specific(os.path.dirname(__file__)))
    yield
    model_path.clear_local_value()


def test_glsl_includes_angle_withdir(gsg, with_current_dir_on_model_path):
    """Test preprocessing includes with angle includes with model-path"""
    suffix = ''
    if gsg.pipe.interface_name == "OpenGL" and \
        (gsg.driver_shader_version_major, gsg.driver_shader_version_minor) < (1, 50):
        suffix = '_legacy'
    vert_path = core.Filename(SHADERS_DIR, 'glsl_include_angle' + suffix + '.vert')
    frag_path = core.Filename(SHADERS_DIR, 'glsl_simple' + suffix + '.frag')
    run_glsl_compile_check(gsg, vert_path, frag_path)
