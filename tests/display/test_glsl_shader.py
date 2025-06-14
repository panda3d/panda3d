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
    imageStore(_triggered, 0, uvec4(0, 0, 0, 0));
    memoryBarrier();
}}

void _assert(bool cond, int line) {{
    if (!cond) {{
        imageStore(_triggered, line, uvec4(1));
    }}
}}

#define assert(cond) _assert(cond, __LINE__)

void main() {{
    _reset();
{body}
}}
"""


def run_glsl_test(gsg, body, preamble="", inputs={}, version=150, exts=set(), state=None):
    """ Runs a GLSL test on the given GSG.  The given body is executed in the
    main function and should call assert().  The preamble should contain all
    of the shader inputs. """

    if not gsg.supports_compute_shaders or not gsg.supports_glsl:
        pytest.skip("compute shaders not supported")

    if not gsg.supports_buffer_texture:
        pytest.skip("buffer textures not supported")

    exts = exts | {'GL_ARB_compute_shader', 'GL_ARB_shader_image_load_store'}
    missing_exts = sorted(ext for ext in exts if not gsg.has_extension(ext))
    if missing_exts:
        pytest.skip("missing extensions: " + ' '.join(missing_exts))

    extensions = ''
    for ext in exts:
        extensions += '#extension {ext} : require\n'.format(ext=ext)

    __tracebackhide__ = True

    preamble = preamble.strip()
    body = body.rstrip().lstrip('\n')
    code = GLSL_COMPUTE_TEMPLATE.format(version=version, extensions=extensions, preamble=preamble, body=body)
    line_offset = code[:code.find(body)].count('\n') + 1
    shader = core.Shader.make_compute(core.Shader.SL_GLSL, code)
    assert shader, code

    # Create a buffer to hold the results of the assertion.  We use one byte
    # per line of shader code, so we can show which lines triggered.
    result = core.Texture("")
    result.set_clear_color((0, 0, 0, 0))
    result.setup_buffer_texture(code.count('\n'), core.Texture.T_unsigned_byte,
                                core.Texture.F_r8i, core.GeomEnums.UH_static)

    # Build up the shader inputs
    attrib = core.ShaderAttrib.make(shader)
    for name, value in inputs.items():
        attrib = attrib.set_shader_input(name, value)
    attrib = attrib.set_shader_input('_triggered', result)

    if not state:
        state = core.RenderState.make(attrib)
    else:
        state = state.set_attrib(attrib)

    # Run the compute shader.
    engine = core.GraphicsEngine.get_global_ptr()
    try:
        engine.dispatch_compute((1, 1, 1), state, gsg)
    except AssertionError as exc:
        assert False, "Error executing compute shader:\n" + code

    # Download the texture to check whether the assertion triggered.
    assert engine.extract_texture_data(result, gsg)
    triggered = result.get_ram_image()
    if any(triggered):
        count = len(triggered) - triggered.count(0)
        lines = body.split('\n')
        formatted = ''
        for i, line in enumerate(lines):
            if triggered[i + line_offset]:
                formatted += '=>  ' + line + '\n'
            else:
                formatted += '    ' + line + '\n'
        pytest.fail("{0} GLSL assertions triggered:\n{1}".format(count, formatted))


def run_glsl_compile_check(gsg, vert_path, frag_path, expect_fail=False):
    """Compile supplied GLSL shader paths and check for errors"""
    shader = core.Shader.load(core.Shader.SL_GLSL, vert_path, frag_path)
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
    tex1 = core.Texture("")
    tex1.setup_1d_texture(1, core.Texture.T_unsigned_byte, core.Texture.F_rgba8)
    tex1.set_clear_color((0, 2 / 255.0, 1, 1))

    tex2 = core.Texture("")
    tex2.setup_2d_texture(1, 1, core.Texture.T_float, core.Texture.F_rgba32)
    tex2.set_clear_color((1.0, 2.0, -3.14, 0.0))

    tex3 = core.Texture("")
    tex3.setup_3d_texture(1, 1, 1, core.Texture.T_float, core.Texture.F_r32)
    tex3.set_clear_color((0.5, 0.0, 0.0, 1.0))

    preamble = """
    uniform sampler1D tex1;
    uniform sampler2D tex2;
    uniform sampler3D tex3;
    """
    code = """
    assert(texelFetch(tex1, 0, 0) == vec4(0, 2 / 255.0, 1, 1));
    assert(texelFetch(tex2, ivec2(0, 0), 0) == vec4(1.0, 2.0, -3.14, 0.0));
    assert(texelFetch(tex3, ivec3(0, 0, 0), 0) == vec4(0.5, 0.0, 0.0, 1.0));
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
    from struct import pack, unpack
    num1 = pack('<i', 1234567)
    num2 = pack('<i', -1234567)
    buffer1 = core.ShaderBuffer("buffer1", num1, core.GeomEnums.UH_static)
    buffer2 = core.ShaderBuffer("buffer2", num2, core.GeomEnums.UH_static)

    preamble = """
    layout(std430, binding=0) buffer buffer1 {
        int value1;
    };
    layout(std430, binding=1) buffer buffer2 {
        int value2;
    };
    """
    code = """
    assert(value1 == 1234567);
    assert(value2 == -1234567);
    value1 = 98765;
    value2 = 5343525;
    """
    run_glsl_test(gsg, code, preamble, {'buffer1': buffer1, 'buffer2': buffer2},
                  exts={'GL_ARB_shader_storage_buffer_object',
                        'GL_ARB_uniform_buffer_object',
                        'GL_ARB_shading_language_420pack'})

    data1 = gsg.get_engine().extract_shader_buffer_data(buffer1, gsg)
    assert unpack('<i', data1[:4]) == (98765, )

    data2 = gsg.get_engine().extract_shader_buffer_data(buffer2, gsg)
    assert unpack('<i', data2[:4]) == (5343525, )


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
    param1 = core.LMatrix4(core.LMatrix3(1, 2, 3, 4, 5, 6, 7, 8, 9))

    param2 = core.NodePath("param2")
    param2.set_mat(core.LMatrix3(10, 11, 12, 13, 14, 15, 16, 17, 18))

    preamble = """
    uniform mat3 param1;
    uniform mat3 param2;
    """
    code = """
    assert(param1[0] == vec3(1, 2, 3));
    assert(param1[1] == vec3(4, 5, 6));
    assert(param1[2] == vec3(7, 8, 9));
    assert(param2[0] == vec3(10, 11, 12));
    assert(param2[1] == vec3(13, 14, 15));
    assert(param2[2] == vec3(16, 17, 18));
    """
    run_glsl_test(gsg, code, preamble, {'param1': param1, 'param2': param2})


def test_glsl_mat4(gsg):
    param1 = core.LMatrix4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16)

    param2 = core.NodePath("param2")
    param2.set_mat(core.LMatrix4(
        17, 18, 19, 20,
        21, 22, 23, 24,
        25, 26, 27, 28,
        29, 30, 31, 32))

    preamble = """
    uniform mat4 param1;
    uniform mat4 param2;
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
    """
    run_glsl_test(gsg, code, preamble, {'param1': param1, 'param2': param2})


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


def test_glsl_pta_mat3(gsg):
    pta = core.PTA_LMatrix3f((
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


def test_glsl_pta_mat4(gsg):
    pta = core.PTA_LMatrix4f((
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
    assert(p3d_LightSource[0].shadowViewMatrix[2][0] == -0.5);
    assert(p3d_LightSource[0].shadowViewMatrix[2][1] == -0.5);
    assert(p3d_LightSource[0].shadowViewMatrix[2][2] > -1.00002);
    assert(p3d_LightSource[0].shadowViewMatrix[2][2] < -1.0);
    assert(p3d_LightSource[0].shadowViewMatrix[2][3] == -1);
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
    if (gsg.driver_shader_version_major, gsg.driver_shader_version_minor) < (1, 50):
        suffix = '_legacy'
    vert_path = core.Filename(SHADERS_DIR, 'glsl_bad' + suffix + '.vert')
    frag_path = core.Filename(SHADERS_DIR, 'glsl_simple' + suffix + '.frag')
    run_glsl_compile_check(gsg, vert_path, frag_path, expect_fail=True)


def test_glsl_from_file(gsg):
    """Test compiling GLSL shaders from files"""
    suffix = ''
    if (gsg.driver_shader_version_major, gsg.driver_shader_version_minor) < (1, 50):
        suffix = '_legacy'
    vert_path = core.Filename(SHADERS_DIR, 'glsl_simple' + suffix + '.vert')
    frag_path = core.Filename(SHADERS_DIR, 'glsl_simple' + suffix + '.frag')
    run_glsl_compile_check(gsg, vert_path, frag_path)


def test_glsl_includes(gsg):
    """Test preprocessing includes in GLSL shaders"""
    suffix = ''
    if (gsg.driver_shader_version_major, gsg.driver_shader_version_minor) < (1, 50):
        suffix = '_legacy'
    vert_path = core.Filename(SHADERS_DIR, 'glsl_include' + suffix + '.vert')
    frag_path = core.Filename(SHADERS_DIR, 'glsl_simple' + suffix + '.frag')
    run_glsl_compile_check(gsg, vert_path, frag_path)


def test_glsl_includes_angle_nodir(gsg):
    """Test preprocessing includes with angle includes without model-path"""
    suffix = ''
    if (gsg.driver_shader_version_major, gsg.driver_shader_version_minor) < (1, 50):
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
    if (gsg.driver_shader_version_major, gsg.driver_shader_version_minor) < (1, 50):
        suffix = '_legacy'
    vert_path = core.Filename(SHADERS_DIR, 'glsl_include_angle' + suffix + '.vert')
    frag_path = core.Filename(SHADERS_DIR, 'glsl_simple' + suffix + '.frag')
    run_glsl_compile_check(gsg, vert_path, frag_path)
